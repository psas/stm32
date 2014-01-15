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

#include "ch.h"
#include "hal.h"


/* PSAS Rocketnet hub board */

static void led_init(void) {

    palClearPad(GPIOD, GPIO_D13_RGB_R);

    palClearPad(GPIOD, GPIO_D11_RGB_B);
    palClearPad(GPIOD, GPIO_D12_RGB_G);

    while (1) {
        palSetPad(GPIOD, GPIO_D13_RGB_R);
        chThdSleepMilliseconds(100);
//        palSetPad(GPIOD, GPIO_D11_RGB_B);
//        chThdSleepMilliseconds(100);
//        palSetPad(GPIOD, GPIO_D12_RGB_G);
//        chThdSleepMilliseconds(100);

        palClearPad(GPIOD, GPIO_D13_RGB_R);
        chThdSleepMilliseconds(100);
//        palClearPad(GPIOD, GPIO_D11_RGB_B);
//        chThdSleepMilliseconds(100);
//        palClearPad(GPIOD, GPIO_D12_RGB_G);
//        chThdSleepMilliseconds(100);
    }
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

  /*
   * Normal main() thread activity, in this demo it enables and disables the
   * button EXT channel using 5 seconds intervals.
   */
    led_init();
    chThdSleepMilliseconds(5000);
    exit(1);
}

