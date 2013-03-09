/*! \file main.c
 *
 * experiments with UDP
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

#include "usb_cdc.h"
#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"

#include "main.h"

static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU1,
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
	static Thread            *shelltp       = NULL;
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
	halInit();
	chSysInit();

	extdetail_init();

	palSetPad(GPIOC, GPIOC_LED);

	/*!
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

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

	/*!
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	chThdSleepMilliseconds(300);


	/*! Activates the EXT driver 1. */
	extStart(&EXTD1, &extcfg);

	chThdCreateStatic(waThread_blinker,      sizeof(waThread_blinker),      NORMALPRIO, Thread_blinker,      NULL);
	chThdCreateStatic(waThread_indwatchdog,  sizeof(waThread_indwatchdog),  NORMALPRIO, Thread_indwatchdog,  NULL);

	chEvtRegister(&extdetail_wkup_event, &el0, 0);
	while (TRUE) {
		if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}


//! @}
