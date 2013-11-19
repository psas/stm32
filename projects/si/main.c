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
#include "ADIS16405.h"
#include "MPL3115A2.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "board.h"

#include "device_net.h"
#include "fc_net.h"

#include "main.h"

#if !defined(DEBUG_SI) || defined(__DOXYGEN__)
#define     DEBUG_SI                   1
#endif

static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};
BaseSequentialStream * chp = NULL;

//
//static void log_msg(volatile char *s) {
//#if DEBUG_SI
//    static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
//    chprintf(chp, "M: %s\r\n", s);
//#else
//    (void) s;
//#endif
//}
//
//static void log_error(volatile char *s) {
//#if DEBUG_SI
//    static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
//    chprintf(chp, "E: %s\r\n", s);
//#else
//    (void) s;
//#endif
//}

/*! \brief ADIS SPI configuration
 *
 * 656250Hz, CPHA=1, CPOL=1, MSb first.
 *
 * For burst mode ADIS SPI is limited to 1Mhz.
 */
#if 1
const SPIConfig adis_spicfg = {
		adis_spi_cb,
		GPIOA,
		4,
		SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
};
#else
const SPIConfig adis_spicfg = {
		NULL,
		GPIOA,
		4,
		SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
};

#endif


/*! \brief ADIS SPI Pin connections
 *
 */
const adis_connect adis_connections = {
		GPIOA,      // spi_sck_port
		5,          // spi_sck_pad;
		GPIOA,      // spi_miso_port;
		6,          // spi_miso_pad;
		GPIOB,      // spi_mosi_port;
		5,          // spi_mosi_pad;
		GPIOA,      // spi_cs_port;
		4,          // spi_cs_pad;
		GPIOD,      // reset_port
		8,          // reset_pad;
		GPIOD,      // dio1_port;
		9,          // dio1_pad;
		GPIOD,      // dio2_port;
		10,         // dio2_pad;
		GPIOD,      // dio3_port;
		11,         // dio3_pad;
		GPIOD,      // dio4_port;
		12          // dio4_pad
};

/*! configure the i2c module on stm32
 *
 */
const I2CConfig mpu9150_config = {
		OPMODE_I2C,
		400000,                // i2c clock speed. Test at 400000 when r=4.7k
		FAST_DUTY_CYCLE_2,
		// STD_DUTY_CYCLE,
};

/*! \typedef mpu9150_config
 *
 * Configuration for the MPU IMU connections
 */
const mpu9150_connect mpu9150_connections = {
		GPIOF,                // i2c sda port
		0,                    // i2c_sda_pad
		GPIOF,                // i2c_scl_port
		1,                    // i2c scl pad
		GPIOF,                // interrupt port
		2,                    // interrupt pad;
};

static void led_init(void) {

    palClearPad(GPIOF, GPIOF_BLUE_LED);
    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_GREEN_LED);

    int i = 0;
    for(i=0; i<5; ++i) {
        palSetPad(GPIOF, GPIOF_RED_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_RED_LED);
        palSetPad(GPIOF, GPIOF_BLUE_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_BLUE_LED);
        palSetPad(GPIOF, GPIOF_GREEN_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_GREEN_LED);
    }
}

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

	rdata = MPU9150_A_HPF_RESET | MPU9150_A_SCALE_pm16g;
	mpu9150_write_accel_config(i2cptr, rdata);

	rdata = MPU9150_G_SCALE_pm2000;
	mpu9150_write_gyro_config(i2cptr, rdata);

	rdata = MPU9150_INT_EN_DATA_RD_EN;
	mpu9150_write_int_enable(i2cptr, rdata);

}

static void mpu9150_reset_req(eventid_t id) {
	(void) id;
	mpu9150_reg_data           rdata;

	rdata = 0;
	mpu9150_write_int_enable(mpu9150_driver.i2c_instance, rdata);

	mpu9150_init(mpu9150_driver.i2c_instance);

}

static void mpu9150_int_event_handler(eventid_t id) {
	(void) id;

	mpu9150_a_read_x_y_z(mpu9150_driver.i2c_instance, &mpu9150_current_read.accel_xyz);

	mpu9150_g_read_x_y_z(mpu9150_driver.i2c_instance, &mpu9150_current_read.gyro_xyz);

	mpu9150_current_read.celsius =  mpu9150_a_g_read_temperature(mpu9150_driver.i2c_instance) ;

	/* broadcast event to data_udp thread for enet transmit to FC */
	chEvtBroadcast(&mpu9150_data_event);

	/* clear the interrupt status bits on mpu9150 */
	mpu9150_a_g_read_int_status(mpu9150_driver.i2c_instance);

#if	DEBUG_MPU9150
	static uint16_t     count = 0;

	++count;
	if (count > 2000) {
        		chprintf(chp, "\r\n*** MPU9150 ***\r\n");
				chprintf(chp, "raw_temp: %d C\r\n", mpu9150_temp_to_dC(mpu9150_current_read.celsius));
				chprintf(chp, "ACCL:  x: %d\ty: %d\tz: %d\r\n", mpu9150_current_read.accel_xyz.x, mpu9150_current_read.accel_xyz.y, mpu9150_current_read.accel_xyz.z);
				chprintf(chp, "GRYO:  x: 0x%x\ty: 0x%x\tz: 0x%x\r\n", mpu9150_current_read.gyro_xyz.x, mpu9150_current_read.gyro_xyz.y, mpu9150_current_read.gyro_xyz.z);
				count = 0;
	}
#endif

}

/*! \brief Periodic reads (post interrupt)
 *
 * Event callback.
 *
 * @param id
 */
static void mpl_read_handler(eventid_t id) {
    (void) id;
    mpl3115a2_i2c_data reg;

    /* Get current data and read f_status to reset INT */
    mpl3115a2_read_P_T(mpl3115a2_driver.i2c_instance, &mpl3115a2_current_read);
    mpl3115a2_read_f_status(mpl3115a2_driver.i2c_instance);

    chEvtBroadcast(&mpl3115a2_data_event);   //! \sa data_udp_send_thread

    /* Set up a one shot which will trigger next interrupt */
    reg =  mpl3115a2_read_ctrl_1(mpl3115a2_driver.i2c_instance);
    reg |= (1<<MPL3115A2_CTL1_OST_BIT);
    mpl3115a2_write_ctrl_1(mpl3115a2_driver.i2c_instance, reg);
}

static WORKING_AREA(waThread_blinker, 64);
/*! \brief Green LED blinker thread
 */
static msg_t Thread_blinker(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

	palClearPad(GPIOF, GPIOF_GREEN_LED);
	palClearPad(GPIOF, GPIOF_RED_LED);
	palClearPad(GPIOF, GPIOF_BLUE_LED);
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
        //palTogglePad(GPIOF, GPIOF_RED_LED);
        //palTogglePad(GPIOF, GPIOF_BLUE_LED);
        palTogglePad(GPIOF, GPIOF_GREEN_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

static WORKING_AREA(waThread_mpu9150_int, 512);
static msg_t Thread_mpu9150_int(void* arg) {
	(void) arg;
	static const evhandler_t evhndl_mpu9150[]       = {
			mpu9150_int_event_handler,
			//mpu9150_reset_req
	};
	struct EventListener     evl_mpu9150;

	chRegSetThreadName("mpu9150_int");

	mpu9150_init(mpu9150_driver.i2c_instance);

	chEvtRegister(&mpu9150_int_event,           &evl_mpu9150,         0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
	}
	return -1;
}


static WORKING_AREA(waThread_mpu9150_reset_req, 256);
static msg_t Thread_mpu9150_reset_req(void* arg) {
	(void) arg;
	static const evhandler_t evhndl_mpu9150[]       = {
			mpu9150_reset_req
	};
	struct EventListener     evl_mpu9150;

	chRegSetThreadName("mpu9150_reset_req");

	chEvtRegister(&fc_req_reset_event,          &evl_mpu9150,         0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
	}
	return -1;
}

static WORKING_AREA(waThread_adis_newdata, 512);
/*! \brief ADIS Newdata Thread
 */
static msg_t Thread_adis_newdata(void *arg) {
	(void)arg;
	chRegSetThreadName("adis_newdata");

	static const evhandler_t evhndl_newdata[]       = {
			adis_newdata_handler
	};
	struct EventListener     evl_spi_cb2;

	chEvtRegister(&adis_spi_cb_data_captured, &evl_spi_cb2, 0);

	while (TRUE) {
		chEvtDispatch(evhndl_newdata, chEvtWaitOneTimeout((eventmask_t)1, US2ST(50)));
	}
	return -1;
}


static WORKING_AREA(waThread_adis_dio1, 128);
/*! \brief ADIS DIO1 thread
 *
 * For burst mode transactions t_readrate is 1uS
 *
 */
static msg_t Thread_adis_dio1(void *arg) {
	(void)arg;
	static const evhandler_t evhndl_dio1[]       = {
			adis_burst_read_handler,
			//adis_read_id_handler,
			//adis_read_dC_handler,
			adis_spi_cb_txdone_handler,
			adis_release_bus
	};
	struct EventListener     evl_dio;
	struct EventListener     evl_spi_ev;
	struct EventListener     evl_spi_release;

	chRegSetThreadName("adis_dio");

	chEvtRegister(&adis_dio1_event,           &evl_dio,         0);
	chEvtRegister(&adis_spi_cb_txdone_event,  &evl_spi_ev,      1);
	chEvtRegister(&adis_spi_cb_releasebus,    &evl_spi_release, 2);

	while (TRUE) {
		chEvtDispatch(evhndl_dio1, chEvtWaitOneTimeout((EVENT_MASK(2)|EVENT_MASK(1)|EVENT_MASK(0)), US2ST(50)));
	}
	return -1;
}

static WORKING_AREA(waThread_mpl_int_1, 256);
/*! \brief MPL INT1 thread
 *
 */
static msg_t Thread_mpl_int_1(void *arg) {
    (void)arg;
    static const evhandler_t evhndl_mplint[] = {
            mpl_read_handler
    };
    struct EventListener     evl_mplint1;

    chRegSetThreadName("mpl_int_1");

    mpl3115a2_init(mpl3115a2_driver.i2c_instance);

    chEvtRegister(&mpl3115a2_int_event,      &evl_mplint1,         0);

    while (TRUE) {
        chEvtDispatch(evhndl_mplint, chEvtWaitOneTimeout((EVENT_MASK(0)), US2ST(50)));
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
	palSetPad(GPIOA, GPIOA_SPI1_SCK);
	palSetPad(GPIOA, GPIOA_SPI1_NSS);

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
	 * SPI1 I/O pins setup.
	 */
	palSetPadMode(adis_connections.spi_sck_port, adis_connections.spi_sck_pad,
			PAL_MODE_ALTERNATE(5) |
			PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(adis_connections.spi_miso_port, adis_connections.spi_miso_pad,
			PAL_MODE_ALTERNATE(5) |
			PAL_STM32_OSPEED_HIGHEST| PAL_STM32_PUDR_FLOATING);
	palSetPadMode(adis_connections.spi_mosi_port, adis_connections.spi_mosi_pad,
			PAL_MODE_ALTERNATE(5) |
			PAL_STM32_OSPEED_HIGHEST );
	palSetPadMode(adis_connections.spi_cs_port, adis_connections.spi_cs_pad,
			PAL_MODE_OUTPUT_PUSHPULL |
			PAL_STM32_OSPEED_HIGHEST);

	palSetPad(GPIOA, GPIOA_SPI1_SCK);
	palSetPad(GPIOA, GPIOA_SPI1_NSS);

	palClearPad(GPIOF, GPIOF_RED_LED);
	palClearPad(GPIOF, GPIOF_BLUE_LED);
	palClearPad(GPIOF, GPIOF_GREEN_LED);

	led_init();

	/*
	 * I2C2 I/O pins setup
	 */
	palSetPadMode(mpu9150_connections.i2c_sda_port , mpu9150_connections.i2c_sda_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |PAL_STM32_PUDR_FLOATING );
	palSetPadMode(mpu9150_connections.i2c_scl_port, mpu9150_connections.i2c_scl_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST  | PAL_STM32_PUDR_FLOATING);


	palSetPad(mpu9150_connections.i2c_scl_port,  mpu9150_connections.i2c_scl_pad );

	usbSerialShellStart(commands);
	chp = getActiveUsbSerialStream();

	iwdg_begin();

	spiStart(&SPID1, &adis_spicfg);       /* Set transfer parameters.  */

	chThdSleepMilliseconds(300);

	adis_init();
	adis_reset();

	mpu9150_start(&I2CD2);
    mpl3115a2_start(&I2CD2);

	i2cStart(mpu9150_driver.i2c_instance, &mpu9150_config);

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

	chThdCreateStatic(wa_lwip_thread              , sizeof(wa_lwip_thread)              , NORMALPRIO + 2, lwip_thread            , &ip_opts);
	chThdCreateStatic(wa_data_udp_send_thread     , sizeof(wa_data_udp_send_thread)     , NORMALPRIO    , data_udp_send_thread   , NULL);
	chThdCreateStatic(wa_data_udp_receive_thread  , sizeof(wa_data_udp_receive_thread)  , NORMALPRIO    , data_udp_receive_thread, NULL);

	/* i2c MPU9150 */
	chThdCreateStatic(waThread_mpu9150_int,         sizeof(waThread_mpu9150_int)        , NORMALPRIO    , Thread_mpu9150_int,        NULL);
	chThdCreateStatic(waThread_mpu9150_reset_req,   sizeof(waThread_mpu9150_reset_req)  , NORMALPRIO    , Thread_mpu9150_reset_req,  NULL);
    chThdCreateStatic(waThread_mpl_int_1,           sizeof(waThread_mpl_int_1)          , NORMALPRIO    , Thread_mpl_int_1,          NULL);

	/* SPI ADIS */
	chThdCreateStatic(waThread_adis_dio1,    sizeof(waThread_adis_dio1),    NORMALPRIO, Thread_adis_dio1,    NULL);
	chThdCreateStatic(waThread_adis_newdata, sizeof(waThread_adis_newdata), NORMALPRIO, Thread_adis_newdata, NULL);


	/*! Activates the EXT driver 1. */
	extStart(&EXTD1, &extcfg);

	chEvtRegister(&extdetail_wkup_event, &el0, 0);
	while (TRUE) {
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}
//! @}
