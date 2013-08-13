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

// bit rate and baud rate
// look for button pressed example
// set it up as an interrupt instead (see extintr) extdetail.c
// edge-sensitive

// defined in os/hal/platforms/STM32/USARTv1/serial_lld.h

extern SerialDriver SD1;  // to laptop
extern SerialDriver SD2;  // to MAX2769

#define SERIAL_BITRATE 9600

// MAX2769 configuration register commands

#define CONF1_DEFAULT     0xA0951A30  // address 0x0
#define CONF2_INDEPENDENT 0x05502881  // address 0x1
#define CONF2_LOCKED      0x0550A881  // address 0x1
#define CONF3_DSPDISABLE  0xEAFE1D62  // address 0x2
#define CONF3_DSPSTART    0xEAFEDDE2  // address 0x2
#define CONF3_DSPSTOP     0xEAFEBDE2  // address 0x2
#define CONFPLL_DEFAULT   0x9EC00083  // address 0x3
#define CONFPLL_INTEGER   0x0C360404  // default 0x4
#define CONFPLL_DIVRATIO  0x80000705  // default 0x5
#define CONFDSP_DEFAULT   0x80000006  // address 0x7
#define CONF_CLKFRAC      0x0FF61B27  // default 0x6
#define CONF_TESTMODE1    0x1E0F4018  // address 0x8
#define CONF_TESTMODE2    0x14C04029  // address 0x9

static const SerialConfig serial_config =
{
		SERIAL_BITRATE,
		0,
		USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		0
};

#if 0
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
#endif

#if 0
/* Simple GPIO read example
 * Read in data from the MAX2769 pin I0 (PB3 on STM32F4)
 * Possibly by way of I/O queue, write that data out the TX (PA9) 
 * on the FTDI output of the GPS RF board.
 * The MAX2769 is set to the default configuration of '000'
 *
 * 16.368 MHz reference frequency
 * 16 reference division ratio
 * 1536 main division ratio
 * I only
 * 1 I Q bit
 * differential I and Q logic level
 * IF center frequency (MHz)
 * 5th order IF filter
 * SCLK = 0
 * SDATA = 0
 * CS = 0
 * 
 * NOTE: going to want the PAL Driver

int main(void) {
    uint32_t i = 0;

    halInit();
    chSysInit();

    uint8_t my_string[10] = "Hello";

}
#endif

#if 0
/* Simple SPI read example
 * SCLK is PB3, MISO is PB4, master is STM32F4 and slave is MAX2769
 * Possibly by way of I/O queue, write that data out the TX (PA9) 
 * on the FTDI output of the GPS RF board.
 * Remember STM32F4 is way faster than MAX2769
 * The MAX2769 is set to the default configuration of '000'
 *
 * Need to set up the SPI in 2769 configuration
 * 
 * 16.368 MHz reference frequency
 * 16 reference division ratio
 * 1536 main division ratio
 * I only
 * 1 I Q bit
 * differential I and Q logic level
 * IF center frequency (MHz)
 * 5th order IF filter
 * SCLK = 0
 * SDATA = 0
 * CS = 0
 * 
 * NOTE: going to want the PAL Driver

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
#endif



