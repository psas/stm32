/*! \file main.c
 *
 * Development for MPU9150 on ChibiOS
 *
 * MPU is an i2c device
 *
 * Includes ADIS SPI connections and development
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */


/*!
 * \defgroup mpu-mainapp MPU Application
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"

#include "MPU9150.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "board.h"

#include "device_net.h"
#include "fc_net.h"

#include "main.h"

/*! configure the i2c module on stm32
 *
 */
const I2CConfig si_i2c_config = {
		OPMODE_I2C,
		400000,                // i2c clock speed. Test at 400000 when r=4.7k
		FAST_DUTY_CYCLE_2,
		// STD_DUTY_CYCLE,
};

/*! \typedef mpu9150_config
 *
 * Configuration for the MPU IMU connections
 */
const mpu9150_connect si_i2c_connections = {
		GPIOF,                // i2c sda port
		0,                    // i2c_sda_pad
		GPIOF,                // i2c_scl_port
		1,                    // i2c scl pad
		GPIOF,                // interrupt port
		2,                    // interrupt pad;
};

/*! \brief Initialize mpu9150
 *
 */

static void mpu9150_init(I2CDriver* i2cptr) {
	mpu9150_reg_data           rdata;

	mpu9150_reset(i2cptr);

	rdata = MPU9150_PM1_X_GYRO_CLOCKREF & (~(MPU9150_PM1_SLEEP));   // make sure device is 'awake'
	mpu9150_write_pm1(i2cptr, rdata);

	rdata = 16;                                          // 2 ms sample period.
	mpu9150_write_gyro_sample_rate_div(i2cptr, rdata);

	rdata = MPU9150_I2C_BYPASS | MPU9150_INT_LEVEL | MPU9150_LATCH_INT_EN;
	mpu9150_write_pin_cfg(i2cptr, rdata);

	rdata = MPU9150_A_HPF_RESET | MPU9150_A_SCALE_pm8g;
	mpu9150_write_accel_config(i2cptr, rdata);

	rdata = MPU9150_G_SCALE_pm500;
	mpu9150_write_gyro_config(i2cptr, rdata);

	rdata = MPU9150_INT_EN_DATA_RD_EN;
	mpu9150_write_int_enable(i2cptr, rdata);
}

static void mpu9150_int_event_handler(eventid_t id __attribute__ ((unused))) {

	mpu9150_a_read_x_y_z(mpu9150_driver.i2c_instance, &mpu9150_current_read.accel_xyz);

	mpu9150_g_read_x_y_z(mpu9150_driver.i2c_instance, &mpu9150_current_read.gyro_xyz);

	mpu9150_current_read.celsius =  mpu9150_a_g_read_temperature(mpu9150_driver.i2c_instance) ;

	/* broadcast event to data_udp thread for enet transmit to FC */
	chEvtBroadcast(&mpu9150_data_event);

	/* clear the interrupt status bits on mpu9150 */
	mpu9150_a_g_read_int_status(mpu9150_driver.i2c_instance);

#if	DEBUG_MPU9150

    static uint16_t     count = 0;

    BaseSequentialStream *chp = getActiveUsbSerialStream();

	++count;
	if (count > 5000) {
				chprintf(chp, "\r\nraw_temp: %d C\r\n", mpu9150_temp_to_dC(mpu9150_current_read.celsius));
				chprintf(chp, "ACCL:  x: %d\ty: %d\tz: %d\r\n", mpu9150_current_read.accel_xyz.x, mpu9150_current_read.accel_xyz.y, mpu9150_current_read.accel_xyz.z);
				chprintf(chp, "GRYO:  x: 0x%x\ty: 0x%x\tz: 0x%x\r\n", mpu9150_current_read.gyro_xyz.x, mpu9150_current_read.gyro_xyz.y, mpu9150_current_read.gyro_xyz.z);
				count = 0;
	}
#endif

}
static WORKING_AREA(waThread_blinker, 64);
/*! \brief Green LED blinker thread
 */
static msg_t Thread_blinker(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

static WORKING_AREA(waThread_mpu9150_int, 512);
static msg_t Thread_mpu9150_int(void* arg) {
	(void) arg;
	static const evhandler_t evhndl_mpu9150[]       = {
			mpu9150_int_event_handler
	};
	struct EventListener     evl_mpu9150;

	chRegSetThreadName("mpu9150_int");

	chEvtRegister(&mpu9150_int_event,           &evl_mpu9150,         0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
	}
	return -1;
}

static WORKING_AREA(waThread_indwatchdog, 64);
/*! \brief  Watchdog thread
 */
static msg_t Thread_indwatchdog(void *arg) {
	(void)arg;

	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(250);
	}
	return -1;
}

int main(void) {
	static const evhandler_t evhndl_main[]       = {
			extdetail_WKUP_button_handler
	};
	struct EventListener     el0;

	struct lwipthread_opts   ip_opts;

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	extdetail_init();

	palSetPad(GPIOC, GPIOC_LED);

	/*!
	 * GPIO Pins for generating pulses at data input detect and data output send.
	 * Used for measuring latency timing of data
	 *
	 * \sa board.h
	 */
	palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
	palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
	palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );

	/*
	 * I2C2 I/O pins setup
	 */
	palSetPadMode(si_i2c_connections.i2c_sda_port , si_i2c_connections.i2c_sda_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |PAL_STM32_PUDR_FLOATING );
	palSetPadMode(si_i2c_connections.i2c_scl_port, si_i2c_connections.i2c_scl_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST  | PAL_STM32_PUDR_FLOATING);


	palSetPad(si_i2c_connections.i2c_scl_port,  si_i2c_connections.i2c_scl_pad );


	static const ShellCommand commands[] = {
	        {"mem", cmd_mem},
	        {"threads", cmd_threads},
	        {NULL, NULL}
	};
	usbSerialShellStart(commands);

	mpu9150_start(&I2CD2);

	i2cStart(mpu9150_driver.i2c_instance, &si_i2c_config);

	mpu9150_init(mpu9150_driver.i2c_instance);

	/* Administrative threads */
	chThdCreateStatic(waThread_blinker,      sizeof(waThread_blinker),      NORMALPRIO, Thread_blinker,      NULL);
	chThdCreateStatic(waThread_indwatchdog,  sizeof(waThread_indwatchdog),  NORMALPRIO, Thread_indwatchdog,  NULL);

	/* MAC */
	/*!
	 * Use a locally administered MAC address second LSbit of MSB of MAC should be 1
	 * Use unicast address LSbit of MSB of MAC should be 0
	 */
	data_udp_init();
	static       uint8_t      IMU_A_macAddress[6]         = IMU_A_MAC_ADDRESS;
	struct       ip_addr      ip, gateway, netmask;
	IMU_A_IP_ADDR(&ip);
	IMU_A_GATEWAY(&gateway);
	IMU_A_NETMASK(&netmask);

	ip_opts.macaddress = IMU_A_macAddress;
	ip_opts.address    = ip.addr;
	ip_opts.netmask    = netmask.addr;
	ip_opts.gateway    = gateway.addr;

	chThdCreateStatic(wa_lwip_thread            , sizeof(wa_lwip_thread)            , NORMALPRIO + 2, lwip_thread            , &ip_opts);
	chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
	chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);

	/* i2c MPU9150 */
	chThdCreateStatic(waThread_mpu9150_int,       sizeof(waThread_mpu9150_int)      , NORMALPRIO    , Thread_mpu9150_int,  NULL);

	/* SPI ADIS */
	//chThdCreateStatic(waThread_adis_dio1,    sizeof(waThread_adis_dio1),    NORMALPRIO, Thread_adis_dio1,    NULL);
	//chThdCreateStatic(waThread_adis_newdata, sizeof(waThread_adis_newdata), NORMALPRIO, Thread_adis_newdata, NULL);


	/*! Activates the EXT driver 1. */
	extStart(&EXTD1, &extcfg);


	chEvtRegister(&extdetail_wkup_event, &el0, 0);
	while (TRUE) {
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}


//! @}
