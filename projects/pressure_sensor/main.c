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
 * \defgroup mainapp Application
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

#include "MPL3115A2.h"

#include "main.h"
#include "board.h"

//#include "lwipopts.h"
//#include "lwipthread.h"

BaseSequentialStream *chp = NULL;

static Mutex mtx;

/*! configure the i2c module on stm32
 *
 */
const I2CConfig mpl3115a2_config = {
    OPMODE_I2C,
    400000,                // i2c clock speed. Test at 400000 when r=4.7k
    FAST_DUTY_CYCLE_2,
    //STD_DUTY_CYCLE,
};

/*! \typedef mpl3115a2_config
 *
 * Configuration for the MPU IMU connections
 */
const mpl3115a2_connect mpl3115a2_connections = {
	GPIOF,                // i2c sda port
	0,                    // i2c_sda_pad
	GPIOF,                // i2c_scl_port
	1,                    // i2c scl pad
	GPIOF,                // interrupt port
	2,                    // interrupt pad;
};

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
/*
static WORKING_AREA(waThread_mpl3115a2_int, 128);
static msg_t Thread_mpl3115a2_int(void* arg) {
	(void) arg;
	static const evhandler_t evhndl_mpl3115a2[]       = {
			mpl3115a2_int_event_handler
	};
	struct EventListener     evl_mpl3115a2;

	chRegSetThreadName("mpl3115a2_int");

	chEvtRegister(&mpl3115a2_int_event,           &evl_mpl3115a2,         0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpl3115a2, chEvtWaitOneTimeout((EVENT_MASK(2)|EVENT_MASK(1)|EVENT_MASK(0)), MS2ST(50)));
	}
	return -1;
}
*/
static WORKING_AREA(waThread_mpl3115a2, 256);
/*! \brief MPU9150 thread
 */
static msg_t Thread_mpl3115a2(void *arg) {
	(void)arg;
	//float altitude = 5.0;
	chRegSetThreadName("mpl3115a2");
	
	MPL3115A2_read_data d;

	while (TRUE) {
		//chEvtDispatch(evhndl_newdata, chEvtWaitOneTimeout((eventmask_t)1, US2ST(50)));
		//chprintf(chp,"This is the pressure sensor thread talking\n");
		//altitude = mpl3115a2_get_altitude(mpl3115a2_driver.i2c_instance);
		chMtxLock(&mtx);
		mpl3115a2_read_P_T(mpl3115a2_driver.i2c_instance, &d);
		chMtxUnlock();
		chprintf(chp,"Temperature: %d\r\n", d.mpu_temperature);
		chprintf(chp, "Pressure %d\r\n",  d.mpu_pressure);
		chThdSleepMilliseconds(1000);
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

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	 chMtxInit(&mtx);
	halInit();
	chSysInit();

	extdetail_init();

	/*
	 * I2C2 I/O pins setup.
	 */
	palSetPadMode(mpl3115a2_connections.i2c_sda_port , mpl3115a2_connections.i2c_sda_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |PAL_STM32_PUDR_FLOATING );
	palSetPadMode(mpl3115a2_connections.i2c_scl_port, mpl3115a2_connections.i2c_scl_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST  | PAL_STM32_PUDR_FLOATING);

	/*
	 * MPU9150 Interrupt pin setup
	 */
	palSetPadMode(mpl3115a2_connections.int_port, mpl3115a2_connections.int_pad,
			PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST| PAL_STM32_PUDR_PULLUP | PAL_STM32_MODE_INPUT);

	palSetPad(mpl3115a2_connections.i2c_scl_port,  mpl3115a2_connections.i2c_scl_pad );


	const ShellCommand commands[] = {
	        {"mem", cmd_mem},
	        {"threads", cmd_threads},
	        {NULL, NULL}
	};
	usbSerialShellStart(commands);
	chp = getActiveUsbSerialStream();

	iwdg_begin();

	//spiStart(&SPID1, &adis_spicfg);       /* Set transfer parameters.  */

	mpl3115a2_start(&I2CD2);
	i2cStart(mpl3115a2_driver.i2c_instance, &mpl3115a2_config);
	chThdSleepMilliseconds(1000);
	mpl3115a2_init(&I2CD2);
	/*! Activates the EXT driver 1. */
	//extStart(&EXTD1, &extcfg);

	chThdCreateStatic(waThread_blinker,      sizeof(waThread_blinker),      NORMALPRIO, Thread_blinker,      NULL);
	chThdCreateStatic(waThread_indwatchdog,  sizeof(waThread_indwatchdog),  NORMALPRIO, Thread_indwatchdog,  NULL);
	chThdCreateStatic(waThread_mpl3115a2,      sizeof(waThread_mpl3115a2),      NORMALPRIO, Thread_mpl3115a2,      NULL);

	chEvtRegister(&extdetail_wkup_event, &el0, 0);
	while (TRUE) {
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}


//! @}
