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

// Minimums from the data sheet
#define TCSS  10  // ns     falling edge of CS to rising edge of first SCLK time
#define TDS   10  // ns     data to serial-clock setup time
#define TDH   10  // ns     data to clock hold time
#define TCH   25  // ns     serial lock pulse-width high
#define TCL   25  // ns     clock pulse-width low
#define TCSH  10  // ns     last sclk rising edge to rising edge of CS 
#define TCSW  1   // clock  CS high pulse width

// Write a word to the 3-wire serial configuration interface 
// Pads: D11 - CSn   D12 - SCLK   D13 - SDATA
// 25ns = 40MHz   10ns = 100 MHz  12MHz = 84ns
// MAX2769 serial clock: Minimum of 40MHz = period of 25 ns
// STM32F4 GPIO clock: 168MHz default = period of 5.95 ns
//
// 168MHz / 8 is 21 MHz = period of 47.6ns 
// 168MHz / 4 is 42 MHz = period of 23.8ns
// 168MHz / 2 is 84 MHz = period of 11.9ns
// 
// wait function takes the number of periods to wait
int max2769_write_word(uint32_t wrd) {

  uint32_t bout;

  palClearPad(GPIOD,11);            // bring CS low
  wait()                            // wait TCSS (10ns min) before sending any data

  for (int i; i < 32; i++) {
    bout = wrd & 0x8000000;         // shift word and set pad D13 to the output bit 
    if (bout) palSetPad(GPIOD,13);  // bit = 1, pad high
    else palClearPad(GPIOD,13);     // bit = 0, pad low
    wait()                          // wait TDS (10ns min)
    palSetPad(GPIOD,12);            // bring clock pad D12 high for TCH 
    wait()                          // wait TDH-TCH (10ns min)
    wrd = wrd << 1;                 // shift the word so MSB is next bit
  }

  wait()                            // wait TCSH (10ns min)
  palSetPad(GPIOD,11);              // bring CS high to end transmission
  wait()                            // wait TCSW before beginning new word write (1 clock)

  return 0;

}

static const SerialConfig serial_config =
{
		SERIAL_BITRATE,
		0,
		USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		0
};

/* Simple Serial example
 * Sets up the GPIO ports for MAX2769
 * Writes some chars out the tx port on the FTDI output of the GPS RF board.
 *
 * GPIO Port A pin 9 is our TX to RS232 FTDI output header
 *
 */
int main(void) {
    uint32_t i = 0;

    halInit();
    chSysInit();

    uint8_t my_string[10] = "Hello";

    // set up the pads for the serial communication
    palSetPadMode(GPIOD, 10, PAL_MODE_INPUT_PULLDOWN);  // ANTFLAG
    palSetPadMode(GPIOC, 6,  PAL_MODE_INPUT_PULLDOWN);  // LD
    palSetPadMode(GPIOD, 15, PAL_MODE_INPUT_PULLUP);    // SHDNn
    palSetPadMode(GPIOD, 9,  PAL_MODE_INPUT_PULLDOWN);  // CLKOUT
    palSetPadMode(GPIOD, 8,  PAL_MODE_INPUT_PULLDOWN);  // Q1
    palSetPadMode(GPIOB, 15, PAL_MODE_INPUT_PULLDOWN);  // Q0
    palSetPadMode(GPIOA, 4,  PAL_MODE_INPUT_PULLUP);    // IDLEn
    palSetPadMode(GPIOD, 10, PAL_MODE_INPUT_PULLDOWN);  // PGM

    // 3-wire configuration interface
    // set up the pads for the 3-wire serial configuration interface
    palSetPadMode(GPIOD, 11, PAL_MODE_OUTPUT_PUSHPULL); // CSn
    palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL); // SCLK
    palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL); // SDATA

    palSetPad(GPIOD, 11);   // chip select is pulled high
    palClearPad(GPIOD, 12); // SClk is pulled low
    palClearPad(GPIOD, 13); // SData is pulled low

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

