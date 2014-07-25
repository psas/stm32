#include "ch.h"
#include "hal.h"
#include "utils_general.h"
#include "utils_led.h"

/* Olimex stm32-e407 board */

static void greenLedOff(void *arg UNUSED) {
	ledOff(&GREEN);
}

/* Triggered when the WKUP button is pressed or released. The LED is set to ON.*/
static void button(EXTDriver *extp UNUSED, expchannel_t channel UNUSED) {
	static VirtualTimer vt;

	ledOn(&GREEN);

	chSysLockFromIsr();
	if (chVTIsArmedI(&vt))
		chVTResetI(&vt);
	chVTSetI(&vt, MS2ST(500), greenLedOff, NULL); // LED set to OFF after 500mS
	chSysUnlockFromIsr();
}

/*
 * Application entry point.
 */
void main(void) {
	halInit();
	chSysInit();

	/* Activates the EXT driver 1. */
	const EXTConfig extcfg =
	{
		{
			{EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, button},
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

	while (TRUE) {
		chThdSleep(TIME_INFINITE);
	}
}
