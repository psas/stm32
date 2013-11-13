/*! \file main.c
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp FATFS experiments
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
#include "cmddetail.h"
#include "pwm_config.h"
#include "board.h"
#include "ff.h"
#include "pwm.h"
#include "main.h"

static const ShellCommand commands[] = {
		{"pwm",     cmd_pwm},
		{"mem",     cmd_mem},
		{"threads", cmd_threads},
		{"phy",     cmd_phy},
		{NULL,      NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU_PSAS,
		commands
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
static WORKING_AREA(waThread_test, 128);
/*! \brief Test thread
 */
static msg_t Thread_test(void *arg) {
	(void)arg;
	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;

	chRegSetThreadName("test");
	while (TRUE) {
		chprintf(chp, "GPIOD_OSPEEDR:\t%d\t0x%x\r\n", GPIOD->OSPEEDR, GPIOD->OSPEEDR);
		chprintf(chp, "GPIOD_OTYPER:\t%d\t0x%x\r\n", GPIOD->OTYPER, GPIOD->OTYPER);
		chprintf(chp, "GPIOD_OODR:\t%d\t0x%x\r\n", GPIOD->ODR, GPIOD->ODR);
		chprintf(chp, "GPIOD_MODER:\t%d\t0x%x\r\n", GPIOD->MODER, GPIOD->MODER);
		chprintf(chp, "GPIOD_PUPDR:\t%d\t0x%x\r\n\r\n", GPIOD->PUPDR, GPIOD->PUPDR);

		//  BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;

	    chThdSleepMilliseconds(2500);
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

/*===========================================================================*/
/* Main and generic code.                                                    */
/*===========================================================================*/

int main(void) {
	static Thread            *shelltp       = NULL;


	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

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

	/*!
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU_PSAS);
	sduStart(&SDU_PSAS, &serusbcfg);

	/*
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	/*!
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	shellInit();

	iwdg_begin();

	chThdSleepMilliseconds(300);
	palSetPad(GPIOC, GPIOC_LED);
//	chThdCreateStatic(waThread_blinker          , sizeof(waThread_blinker)          , NORMALPRIO    , Thread_blinker         , NULL);
	chThdCreateStatic(waThread_indwatchdog      , sizeof(waThread_indwatchdog)      , NORMALPRIO    , Thread_indwatchdog     , NULL);
	chThdCreateStatic(waThread_test      , sizeof(waThread_test)      , NORMALPRIO    , Thread_test     , NULL);

	pwm_start();

    while (TRUE) {
		if (!shelltp && (SDU_PSAS.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chThdSleepMilliseconds(50);
		//chEvtDispatch(evhndl_main, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(50)));
	}
}


//! @}
