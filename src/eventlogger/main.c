#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "rtc.h"

#include "psas_rtc.h"
#include "sdcdetail.h"
#include "usbdetail.h"

#include "eventlogger.h"



/*
 * Event Generator Thread
 */

static WORKING_AREA(wa_thread_event_generator, 64);

msg_t event_generator(void *_) {
  char i;

  chThdSleepMilliseconds(2000);
  for (i = 0; i < 255; i++) {
    post_event((event_t) i);
    if (i % 10 == 0) chThdSleepMilliseconds(500);
  }

  return 0;
}


/*
 * LED Blinker Thread
 *
 * Blink slowly when the SD card is not inserted, and quickly when it is.
 */

static WORKING_AREA(wa_thread_blinker, 64);

static msg_t blinker(void *_) {
  chRegSetThreadName("blinker");

  while (TRUE) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(fs_ready ? 125 : 500);
  }

  return -1;
}


/*
 * Main
 */

int main(void) {
  static const evhandler_t sdc_handlers[] = { InsertHandler, RemoveHandler };
  struct EventListener insertion_listener, removal_listener;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  psas_rtc_lld_init();

  palSetPad(GPIOC, GPIOC_LED);

  /*
   * Initializes serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU_PSAS);
  sduStart(&SDU_PSAS, &serusbcfg);

  /*!
   * Activates the serial driver 6 and SDC driver 1 using default
   * configuration.
   */
  sdStart(&SD6, NULL);
	sdcStart(&SDCD1, NULL);

  /*
   * Activates SD card insertion monitor & registers SD card events.
   */
  sdc_init(&SDCD1);
  chEvtRegister(&sdc_inserted_event, &insertion_listener, 0);
  chEvtRegister(&sdc_removed_event,  &removal_listener,   1);

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
   * Starts the LED blinker thread
   */
  chThdCreateStatic( wa_thread_blinker
                   , sizeof(wa_thread_blinker)
                   , NORMALPRIO
                   , blinker
                   , NULL
                   );

  /*!
   * Starts the eventlogger thread and our own event generator thread.
   */
  eventlogger_init();
  chThdCreateStatic( wa_thread_event_generator
                   , sizeof(wa_thread_event_generator)
                   , NORMALPRIO
                   , event_generator
                   , NULL
                   );

  /*
   * Wait for events to fire
   */
  while (true) {
    chEvtDispatch(sdc_handlers, chEvtWaitOne(ALL_EVENTS));
  }
  exit(0);
}
