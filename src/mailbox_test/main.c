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
#include "chprintf.h"

#include "usbdetail.h"

#define MB_SIZE 32


msg_t mail_buffer[MB_SIZE];

static MAILBOX_DECL(my_mail, mail_buffer, MB_SIZE);

void mail_singlethread_test(void) {
  chMBPost(&my_mail, 'A', TIME_IMMEDIATE);
  msg_t fetched_msg;
  msg_t fetch_status = chMBFetch(&my_mail, &fetched_msg, TIME_IMMEDIATE);

  if (fetch_status != RDY_OK) {
    // figure out how to print out an error here
  }

  if (fetched_msg != 'A') {
    // figure out how to print out an error here
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
   * Initializes serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU_PSAS);
  sduStart(&SDU_PSAS, &serusbcfg);

	/*!
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	/*!
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	chThdSleepMilliseconds(300);
  /*
   * Normal main() thread activity,
   */
  while (1) {
    chprintf((BaseSequentialStream *) &SDU_PSAS, "hello over USB Serial from the STM32!!\r\n");
    chThdSleep(MS2ST(1000));
  }
  exit(0);
}

