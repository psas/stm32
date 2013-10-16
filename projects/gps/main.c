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

#define GPIOA_PIN11 GPIOA_OTG_FS_DM

static const SerialConfig serial_config =
{
		SERIAL_BITRATE,
		0,
		USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		0
};

/*
 * A simple print a string function when you don't have printf...
 */
static void my_println(char *p) {
	while (*p) {
		chSequentialStreamPut(&SD1, *p++);
	}
	// chSequentialStreamWrite(&SD1, (uint8_t *)"\n", 2);
}

static void lcd_scroll_right(void) {
	chSequentialStreamPut(&SD1, 0xfe);
	chSequentialStreamPut(&SD1, 0x1c);
	chThdSleepMilliseconds(750);

}

static void lcd_clear_screen(void) {
	chSequentialStreamPut(&SD1, 0xfe);
	chSequentialStreamPut(&SD1, 0x01);
}
/* Simple Serial example
 * Write some chars out the tx port on the e407 board.
 *
 * CON3 pin D1 is TX. Also known as GPIO Port B, pin 6
 */
int main(void) {
	uint32_t    i          = 0;

    halInit();
    chSysInit();

    char	my_string[59] = "I am a good     gadget.";

    /*
     * Don't try this at home!
     *
     * This makes chprintf work with SerialDriver Type.
     * This is an example of TYPE ABUSE and should be looked at very closely.
     *
     * See io_channel.h : _base_channel_methods
     * See chstreams.h  : _base_sequential_stream_methods
     *
     * It's just luck. If this were real C++, you'd get a compile error. (We hope).
     *
     * ...But tracing why this is bad is a good way to learn about how Chibios is put
     *    together.
     */
  //  BaseSequentialStream* chp = (BaseSequentialStream *)&SD1;

    /* Alternate pad mode is 7 for this port / pin.
     *
     * See the datasheet-NOT the reference manual
     */
    palSetPadMode(GPIOB, GPIOB_PIN6, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOB, GPIOB_PIN7, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, GPIOA_PIN11, PAL_MODE_OUTPUT_PUSHPULL);

    /* activate serial driver 1
     * default configuration ('NULL' means use default.)
     */
    //sdStart(&SD1, &serial_config); // default setup is 38400 8N1 for Chibios

    while (TRUE) {
    	palTogglePad(GPIOA, GPIOA_PIN11);
    	chThdSleepMilliseconds(500);
#if 0
        //chThdSleepMilliseconds(5);
        for( i = 0; i < 1000000; i++ ) {

        }


        palClearPort(GPIOA, GPIOA_PIN11);
//        chThdSleepMilliseconds(5);
        for(i = 0; i < 1000000; i++ ) {

                }
        palSetPort(GPIOA, GPIOA_PIN11);
#endif
    }

#if 0
    while (TRUE) {
    	chThdSleepMilliseconds(750);
    	lcd_clear_screen();
    	//chprintf(chp, "%s fn. ", __func__);
    	my_println(my_string);
    	chThdSleepMilliseconds(1500);
    	for(i=0; i<16; ++i) {
    		lcd_scroll_right();
    	}
    }
#endif
}

