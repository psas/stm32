/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "usb_cdc.h"
#include "chprintf.h"
#include "shell.h"

#include "usbdetail.h"
#include "pwmdetail.h"

#include "main.h"

/* Olimex stm32-e407 board */

static EventSource wkup_event;

/*! \sa HAL_USE_EXT in hal_conf.h
 */
static void green_led_off(void *arg) {
	(void)arg;
	palSetPad(GPIOC, GPIOC_LED);
}

/* Triggered when the WKUP button is pressed or released. The LED is set to ON.*/

/* Challenge: Add de-bouncing */
static void extcb1(EXTDriver *extp, expchannel_t channel) {
	static VirtualTimer vt4;

	(void)extp;
	(void)channel;

	palClearPad(GPIOC, GPIOC_LED);
	chSysLockFromIsr();
	chEvtBroadcastI(&wkup_event);

	if (chVTIsArmedI(&vt4))
		chVTResetI(&vt4);

	/* LED4 set to OFF after 500mS.*/
	chVTSetI(&vt4, MS2ST(500), green_led_off, NULL);
	chSysUnlockFromIsr();
}

static const EXTConfig extcfg = {
		{
				{EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1},   // WKUP Button PA0
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL}
		}
};


/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
	static const char *states[] = {THD_STATE_NAMES};
	Thread *tp;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: threads\r\n");
		return;
	}
	chprintf(chp, "    addr    stack prio refs     state time\r\n");
	tp = chRegFirstThread();
	do {
		chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
				(uint32_t)tp, (uint32_t)tp->p_ctx.r13,
				(uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
				states[tp->p_state], (uint32_t)tp->p_time);
		tp = chRegNextThread(tp);
	} while (tp != NULL);
}

/*
 * Challenge: add additional command line functions
 */
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
 * PWM working thread
 */
static WORKING_AREA(waThread2, 128);
static msg_t Thread2(void *arg) {
	unsigned int      pwm_period_new = INIT_PWM_PERIOD;
	bool              upcount        = TRUE;
	(void)arg;
	chRegSetThreadName("pwm_experiment");
	while (TRUE) {
		if(upcount) {
			pwm_period_new += 1;
		} else {
			pwm_period_new -= 1;
		}

		if(pwm_period_new > (INIT_PWM_PERIOD * 0.5)) {
			upcount = FALSE;
		}
		if(pwm_period_new < (INIT_PWM_WIDTH_TICS+1)) {
			upcount = TRUE;
		}
		pwmChangePeriod(&PWMD4, pwm_period_new);
		chThdSleepMilliseconds(20);
	}
	return -1;
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
 * WKUP button handler
 *
 * Challenge: Do something more interesting here.
 */
static void WKUP_button_handler(eventid_t id) {
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;
	chprintf(chp, "WKUP btn. eventid: %d\r\n", id);
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

	/*
	 * Initialize event structures BEFORE using them
	 */
	chEvtInit(&wkup_event);

	/*
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

	/*
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	/*
	 * Shell manager initialization.
	 */
	shellInit();

	/* Enables PWM output (of TIM4, channel 4)  */
//	palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2)); // Set this in ./boards/OLIMEX_STM32_E407/board.h
	pwmStart(&PWMD4, &pwmcfg_led);
	pwmEnableChannel(&PWMD4, 3, INIT_PWM_WIDTH_TICS);

	/*
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	/*
	 * Activates the EXT driver 1.
	 * This is for the external interrupt
	 */
	extStart(&EXTD1, &extcfg);

	/*
	 * Creates the blinker thread.
	 */
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	/*
	 * Creates the pwm thread.
	 */
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

	/*
	 * Normal main() thread activity, in this demo it does nothing except
	 * sleeping in a loop and listen for events.
	 */
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
