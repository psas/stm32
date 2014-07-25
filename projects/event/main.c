/* Demo of using ChibiOS' event system */
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

#include "utils_general.h"
#include "utils_led.h"
#include "usbdetail.h"

/* Event source declaration using a macro to statically initialize it */
static EVENTSOURCE_DECL(button_event);
static EVENTSOURCE_DECL(shell_event);
/* Try it out yourself! uncomment the following event and using a virtual timer,
 * try to implement a periodic timer event that prints a message.
 */
//static EVENTSOURCE_DECL(timer_event);


static void cmd_event(BaseSequentialStream *chp, int argc UNUSED, char *argv[] UNUSED){
	chprintf(chp, "Sending shell event \r\n");
	/* Call the normal version because we're not in an ISR */
	chEvtBroadcast(&shell_event);
}

/* Triggered when the button is pressed or released. The LED is toggled.*/
static void ext_cb(EXTDriver *extp UNUSED, expchannel_t channel UNUSED) {

	ledToggle(&GREEN);

	/* It is good practice to invoke this API before invoking any I-class
	 * syscall from an interrupt handler.
	 */
	chSysLockFromIsr();
	/* Call the I-Class version because we're in an ISR */
	chEvtBroadcastI(&button_event);
	chSysUnlockFromIsr();
}

static void button_handler(eventid_t id) {
	BaseSequentialStream *chp = getUsbStream();
	chprintf(chp, "Button eventid: %d\r\n", id);
}

static void shell_handler(eventid_t id){
	BaseSequentialStream *chp = getUsbStream();
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

	/* As far as I can tell, each event source needs 0 or more unique event listeners
	 * - that is event listeners can't be shared.
	 */

	struct EventListener el0, el1;
	chEvtRegister(&button_event, &el0, 0);
	chEvtRegister(&shell_event, &el1, 1);

	/* Start the USB serial shell */
	static const ShellCommand commands[] = {
		{"event", cmd_event},
		{NULL, NULL}
	};

	usbSerialShellStart(commands);

	/* Activate the external interrupt driver */
	palSetPadMode(GPIOA, GPIOA_BUTTON_WKUP, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	static EXTConfig extcfg =
	{
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
		chEvtDispatch(evhndl, chEvtWaitAll(ALL_EVENTS));
	}
}
