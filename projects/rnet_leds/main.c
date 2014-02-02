
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
    led_init();
}

