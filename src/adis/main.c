/*! \file main.c
 *
 * Development for ADIS IMU on ChibiOS
 *
 * This implementation is specific to the Olimex stm32-e407 board.
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

EventSource   wkup_event;
EventSource   spi1_event;

static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU1,
		commands
};

/*
 * WKUP button handler
 *
 */
static void WKUP_button_handler(eventid_t id) {
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;
	chprintf(chp, "WKUP btn. eventid: %d\r\n", id);
}

/*
 * SPI1 event handler
 *
 */
static void SPI1_handler(eventid_t id) {
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;
	chprintf(chp, "SPI1. eventid: %d\r\n", id);
}

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

/*
 * SPI1 thread
 *
 * For burst mode transactions t_readrate is 1uS
 * Waking every 5 us to look for interrupt seems ok for a start.
 *
 */
static WORKING_AREA(waThread2, 128);
static msg_t Thread2(void *arg) {
	(void)arg;
	static const evhandler_t evhndl_spi1[]       = {
			SPI1_handler
	};
	struct EventListener     evl_spi0;

	chEvtRegister(&spi1_event, &evl_spi0, 0);

	chRegSetThreadName("spi1_adis");
	while (TRUE) {
		chThdSleepMicroseconds(5);
		chEvtDispatch(evhndl_spi1, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
	}
	return -1;
}

/*
 * Watchdog thread
 */
static WORKING_AREA(waThread3, 64);
static msg_t Thread3(void *arg) {
	(void)arg;

	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(250);
	}
	return -1;
}

/*! \brief check reset status and then start iwatchdog
 *
 * Check the CSR register for reset source then start
 * the independent watchdog counter.
 *
 */
static void adis_begin_iwdg(void) {
	// was this a reset caused by the iwdg?
	if( (RCC->CSR & RCC_CSR_WDGRSTF) != 0) {
		// \todo Log WDG reset event somewhere.
		RCC->CSR = RCC->CSR | RCC_CSR_RMVF;  // clear the IWDGRSTF
	}
	iwdg_lld_set_prescale(IWDG_PS_DIV8); // This should be about 1 second at 32kHz
	iwdg_lld_reload();
	iwdg_lld_init();
}

/*
 * Application entry point.
 */
int main(void) {
	static Thread            *shelltp       = NULL;
	static const evhandler_t evhndl[]       = {
			WKUP_button_handler
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

	chEvtInit(&wkup_event);
	chEvtInit(&spi1_event);

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

	adis_begin_iwdg();

	/*!
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	/*!
	 * Activates the EXT driver 1.
	 * This is for the external interrupt
	 */
	extStart(&EXTD1, &extcfg);

	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
	chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);

	chEvtRegister(&wkup_event, &el0, 0);
	while (TRUE) {
		if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
	}
}
