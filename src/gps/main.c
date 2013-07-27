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

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

extern SerialDriver SD1; // defined in os/hal/platforms/STM32/USARTv1/serial_lld.h

#define SERIAL_BITRATE 9600

static const SerialConfig serial_config =
{
		SERIAL_BITRATE,
		0,
		USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		0
};

/* Simple Serial example
 * Write some chars out the tx port on the FTDI output of the GPS RF board.
 *
 * GPIO Port A pin 9 is our TX
 */
int main(void) {
    uint32_t i = 0;

    halInit();
    chSysInit();

    uint8_t my_string[10] = "Hello";

    /* Alternate pad mode is 7 for this port / pin.
     *
     * See the datasheet-NOT the reference manual
     */
      palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));  // RX
      palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));   // TX

    /* activate serial driver 1
     * default configuration ('NULL' means use default.)
     */
    sdStart(&SD1, &serial_config); // default setup is 38400 8N1 for Chibios

    while (TRUE) {
      sdWrite(&SD1,my_string,strlen(my_string));
      sdWrite(&SD1,"\n\r",2);
      sdWrite(&SD1,"Hello, world!\n\r",15);
      chThdSleepMilliseconds(1000);
    }
}

