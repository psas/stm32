/*
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbdetail.h"


/* Olimex e407 board */

BaseSequentialStream * chp = NULL;

static void led(void) {
  chp = getUsbStream();

    while (1) {
  chprintf(chp, "hello over USB Serial from the STM32!!\r\n");

        palTogglePad(GPIOC, GPIOC_LED);
        chThdSleepMilliseconds(500);
    }
}


/*
 * Application entry point.
 */
void main(void) {

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations. Config files for these
     *   are located in halconf.h and mcuconf.h
     */
    halInit();
    /*
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active. The kernel config file is located in chconf.h
     */
    chSysInit();

    /*
     * Normal main() thread activity, in this demo it unendingly blinks an LED
     */
    led();


    /*
     * main() should never return
     */
    while(TRUE){
        chThdSleep(TIME_INFINITE);
    }
}

