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

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "gpt.h"

// See output on: PIN_AFIO_AF(GPIOD_PIN14_GPT_T3, 2) |


/*
 * GPT3 callback.
 */
static void gpt3cb(GPTDriver *gptp) {

	(void)gptp;
	palTogglePad(GPIOC, GPIOC_LED);
	//  palTogglePad(GPIOD, GPIOD_PIN14_GPT_T3);

}


/*
 * GPT3 configuration.
 */
static const GPTConfig gpt3cfg = {
		10000,    /* 10kHz timer clock.*/
		gpt3cb      /* Timer callback.*/  // NULL for no callback?
};

static WORKING_AREA(waThread_25mhz, 64);
/*! \brief 25 Mhz output clock ... or close
 *
 * This is a hack to generate a 25Mhz clock using
 * nop commands between setting and clearing the output
 * buffer. It runs about 27.9Mhz.
 *
 * This is due to failure to use timer to generate more
 * precise output clocks.
 */
static msg_t Thread_25mhz(void *arg) {
	(void)arg;

	while(TRUE) {
		palClearPad(GPIOD, GPIOD_PIN14_GPT_T3);
		asm volatile("mov r0, r0");

		palSetPad(GPIOD, GPIOD_PIN14_GPT_T3);
		asm volatile("mov r0, r0");

	}
	return -1;
}



/*
 * Application entry point.
 */
int main(void) {

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();


	chThdCreateStatic(waThread_25mhz    , sizeof(waThread_25mhz)            , NORMALPRIO    , Thread_25mhz           , NULL);

	/*
	 * Initializes the GPT driver 3.
	 */

	gptStart(&GPTD4, &gpt3cfg);
	gptPolledDelay(&GPTD4, 10); /* Small delay.*/

	gptStartContinuous(&GPTD4, 1000);
	while (TRUE) {



		//   palClearPad(GPIOC, GPIOC_LED);
		// gptStartContinuous(&GPTD4, 5000);
		chThdSleepMilliseconds(1000);
		// gptStopTimer(&GPTD4);
		//   palSetPad(GPIOC, GPIOC_LED);
		// gptStartContinuous(&GPTD3, 5000);
		// chThdSleepMilliseconds(1000);
		//  gptStopTimer(&GPTD3);
	}
	exit(1);
}
