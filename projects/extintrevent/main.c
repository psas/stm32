
#include "ch.h"
#include "hal.h"

#include "usbdetail.h"
#include "chprintf.h"
#include "shell.h"
#include "cmddetail.h"

static EventSource button_event;
static EventSource shell_event;
//static EventSource timer_event; ?


void cmd_event(BaseSequentialStream *chp, int argc, char *argv[]){
    chprintf(chp, "Sending shell event \r\n");
    /* Call the normal version because we're not in an ISR */
    chEvtBroadcast(&shell_event);
}

/* Triggered when the button is pressed or released. The LED is toggled.*/
static void ext_cb(EXTDriver *extp, expchannel_t channel) {

  (void)extp;
  (void)channel;

  palTogglePad(GPIOC, GPIOC_LED);

  /* It is good practice to invoke this API before invoking any I-class
   * syscall from an interrupt handler.
   */
  chSysLockFromIsr();
  /* Call the I-Class version because we're in an ISR */
  chEvtBroadcastI(&button_event);
  chSysUnlockFromIsr();
}

static void button_handler(eventid_t id) {
	BaseSequentialStream *chp = getActiveUsbSerialStream();
	chprintf(chp, "Button eventid: %d\r\n", id);
}

static void shell_handler(eventid_t id){
    BaseSequentialStream *chp = getActiveUsbSerialStream();
    chprintf(chp, "Shell eventid: %d\r\n", id);
}

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

  /* Initialize event structures BEFORE using them */
  chEvtInit(&button_event);
  chEvtInit(&shell_event);

  /* As far as I can tell, each event source needs 0 or more unique event listeners
   * - that is event listeners can't be shared.
   */

  struct EventListener el0, el1;
  chEvtRegister(&button_event, &el0, 0);
  chEvtRegister(&shell_event, &el1, 1);

  /* Start the USB serial shell */
  static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"event", cmd_event},
    {NULL, NULL}
  };

  usbSerialShellStart(commands);

  /* Activate the external interrupt driver */
  static EXTConfig extcfg = {
    {
      {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, ext_cb},   // WKUP Button PA0
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

  /* Run the event loop */
  const evhandler_t evhndl[] = {
    button_handler,
    shell_handler
  };
  while (TRUE) {
    chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
  }
}
