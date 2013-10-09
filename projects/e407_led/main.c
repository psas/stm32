/*
*/

#include "ch.h"
#include "hal.h"


/* Olimex e407 board */

static void led_init(void) {

    palClearPad(GPIOC, GPIOC_LED);

    while (1) {
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


  /*
   * main() should never return
   */
  while(TRUE){
      chThdSleep(TIME_INFINITE);
  }
}

