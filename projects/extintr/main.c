#include "ch.h"
#include "hal.h"


/* Olimex stm32-e407 board */

/*! \sa HAL_USE_EXT in hal_conf.h
 */

static void green_led_off(void *arg __attribute__ ((unused))) {
  palSetPad(GPIOC, GPIOC_LED);
}

/* Triggered when the WKUP button is pressed or released. The LED is set to ON.*/
static void extcb1(EXTDriver *extp __attribute__((unused)),
        expchannel_t channel __attribute__((unused))) {

    static VirtualTimer vt4;

  palClearPad(GPIOC, GPIOC_LED);
  chSysLockFromIsr();
  if (chVTIsArmedI(&vt4))
    chVTResetI(&vt4);

  /* LED4 set to OFF after 500mS.*/
  chVTSetI(&vt4, MS2ST(500), green_led_off, NULL);
  chSysUnlockFromIsr();
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
   * Activates the EXT driver 1.
   */

  const EXTConfig extcfg = {
      {
          {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL},
          {EXT_CH_MODE_DISABLED, NULL}
      }
  };

  extStart(&EXTD1, &extcfg);

  /*
   * Normal main() thread activity, in this demo it enables and disables the
   * button EXT channel using 5 seconds intervals.
   */
  while (TRUE) {
//    chThdSleepMilliseconds(5000);
//    extChannelDisable(&EXTD1, 0);
    chThdSleep(5);
    extChannelEnable(&EXTD1, 0);
  }
}
