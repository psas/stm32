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
#include <stdbool.h>
#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "cmddetail.h"

/*! The goal of this code is to run the shell through the serial terminal
 * and not the usb subsystem. Connect an FTDI serial/usb connector to the
 * appropriate (in this case c6/c7 pins.)
 *
 * On the e407 the serial6 pins on portc6,7 are found on the UEXT connector.
 * See the schematic for pinout.
 *     stm		uext      ftdi
 * TX: pc6      pin3      rx (yellow)
 * RX: pc7      pin4      tx (orange)
 *
 * In the mcuconf.h file enable the proper serial subsystem:
 * #define STM32_SERIAL_USE_USART6             TRUE
 *
 * In the halconf.h enable the serial system
 * #define HAL_USE_SERIAL                      TRUE
 */


static uint32_t           led_wait_time         =        500;

static const ShellCommand commands[] = {
		{"phy_read", cmd_phy_read},
		{"find_phy", cmd_find_phy},
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SD6,
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
		chThdSleepMilliseconds(led_wait_time);
	}
	return -1;
}


/*
 * Application entry point.
 */
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

	// start the serial port
	sdStart(&SD6, NULL);

	/*
	 * Shell manager initialization.
	 */
	shellInit();
	chThdCreateStatic(waThread_blinker          , sizeof(waThread_blinker)          , NORMALPRIO    , Thread_blinker         , NULL);
	/*
	 * Normal main() thread activity, in this demo it enables and disables the
	 * button EXT channel using 5 seconds intervals.
	 */

	while (true) {
		if (!shelltp )
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chThdSleepMilliseconds(1000);
	}

	exit(0);
}

