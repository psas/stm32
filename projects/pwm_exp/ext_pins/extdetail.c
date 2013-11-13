/*! \file extdetail.c
 *
 */

/*!
 * \defgroup extdetail EXT Utilities
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "usbdetail.h"
#include "extdetail.h"

EventSource     extdetail_wkup_event;

/*! \sa HAL_USE_EXT in hal_conf.h
 */
const EXTConfig extcfg = {
		{
				{EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extdetail_wkup_btn},   // WKUP Button PA0
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

/*!
 * Initialize event for wakup button on olimex board.
 */
void extdetail_init() {
	chEvtInit(&extdetail_wkup_event);
}

static void green_led_off(void *arg) {
	(void)arg;
	palSetPad(GPIOC, GPIOC_LED);
}

/*!
 * WKUP button handler
 *
 * Used for debugging
 */
void extdetail_WKUP_button_handler(eventid_t id) {
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
	chprintf(chp, "\r\nWKUP btn. eventid: %d\r\n", id);
}

/*! Triggered when the WKUP button is pressed or released. The LED is set to ON.
 *
 * Challenge: Add de-bouncing
 */
void extdetail_wkup_btn(EXTDriver *extp, expchannel_t channel) {
	static VirtualTimer vt4;

	(void)extp;
	(void)channel;

	palClearPad(GPIOC, GPIOC_LED);
	chSysLockFromIsr();
	chEvtBroadcastI(&extdetail_wkup_event);

	if (chVTIsArmedI(&vt4))
		chVTResetI(&vt4);

	/* LED4 set to OFF after 500mS.*/
	chVTSetI(&vt4, MS2ST(500), green_led_off, NULL);
	chSysUnlockFromIsr();
}

//! @}

