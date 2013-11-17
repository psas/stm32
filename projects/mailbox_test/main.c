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
#define TEST_MB_MSG_LENGTH 13

BaseSequentialStream * chp = NULL;

msg_t mail_buffer[MB_SIZE];

static MAILBOX_DECL(my_mail, mail_buffer, MB_SIZE);

static bool mail_singlethread_test(void) {
  chMBPost(&my_mail, 'A', TIME_IMMEDIATE);
  msg_t fetched_msg;
  msg_t fetch_status = chMBFetch(&my_mail, &fetched_msg, TIME_IMMEDIATE);

  if (fetch_status != RDY_OK) {
    // figure out how to print out an error here
    return false;
  }

  if (fetched_msg != 'A') {
    // figure out how to print out an error here
    return false;
  }

  return true;
}

static char *TEST_MESSAGE = "HELLO WORLD!";
static WORKING_AREA(waThread_Test_Sender, 64);

static msg_t mail_multithread_test_sender(void *_) {
  msg_t status;
  int i;

  for (i = 0; i < TEST_MB_MSG_LENGTH; i++) {
    //chprintf(chp, "sending test message #%d\r\n", i);
    status = chMBPost(&my_mail, TEST_MESSAGE[i], TIME_IMMEDIATE);
    if (status != RDY_OK) {
      //chprintf(chp, "could not send test message #%d\r\n", i);
    }
  }

  return 0;
}

static msg_t mail_test_buffer[TEST_MB_MSG_LENGTH];
static WORKING_AREA(waThread_Test_Receiver, 512);

static msg_t mail_multithread_test_receiver(void *_) {
  int i;

  for (i = 0; i < TEST_MB_MSG_LENGTH; i++) {
    chprintf(chp, "waiting for test message\r\n", i);
    chMBFetch(&my_mail, &mail_test_buffer[i], TIME_INFINITE);
  }

  for (i = 0; i < TEST_MB_MSG_LENGTH; i++) {
    if (mail_test_buffer[i] != TEST_MESSAGE[i]) {
      chprintf(chp, "test message #%d is incorrect!\r\n", i);
      chprintf(chp, "expecting #%c, got #%c\r\n", i);
      return 1;
    }
  }

  chprintf(chp, "all test messages received as expected!\r\n");

  return 0;
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

  chp = getActiveUsbSerialStream();

	chThdSleepMilliseconds(1300);


  /*
   * Spawn the test threads
   */
	chThdCreateStatic( waThread_Test_Receiver
                   , sizeof(waThread_Test_Receiver)
                   , NORMALPRIO
                   , mail_multithread_test_receiver
                   , NULL
                   );
	chThdCreateStatic( waThread_Test_Sender
                   , sizeof(waThread_Test_Sender)
                   , NORMALPRIO
                   , mail_multithread_test_sender
                   , NULL
                   );

  /*
   * Normal main() thread activity,
   */
  while (1) {
//    chprintf(chp, "hello over USB Serial from the STM32!!\r\n");
    chThdSleep(MS2ST(1000));
  }
  exit(0);
}

