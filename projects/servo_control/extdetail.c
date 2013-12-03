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

#include "data_udp.h"
#include "usbdetail.h"
#include "extdetail.h"

bool                           launch_detected = false;

static       uint16_t          extdetail_wkup_button_pressed = 0;

EventSource                    extdetail_wkup_event;
EventSource                    extdetail_launch_detect_event;

/*! \sa HAL_USE_EXT in hal_conf.h
 */
const EXTConfig extcfg = {
		{
				{EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extdetail_wkup_btn},            // WKUP Button PA0
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL}, // D9
				{EXT_CH_MODE_DISABLED, NULL}, // D10
				{EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOD, extdetail_launch_detect},   // D11
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL}, // 14 R PF3 G PF2 B PF14
				{EXT_CH_MODE_DISABLED, NULL}, // 15
				{EXT_CH_MODE_DISABLED, NULL}, //
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
    chEvtInit(&extdetail_launch_detect_event);
}

/*!
 * Initialize event for launch detect pin on olimex board.
 *
 * Launch detect info must go to FC
 */
void launch_detect_init() {

    int     pinval1, pinval2;

    launch_detected = false;

    pinval1 = palReadPad(GPIOD, 11);
    chThdSleepMilliseconds(10);
    pinval2 = palReadPad(GPIOD, 11);

    if((pinval1 == PAL_LOW) && (pinval2 == PAL_LOW)) {
        launch_detected = true;
    } else {
        launch_detected = false;
    }
}

/*!
 * WKUP button handler
 *
 * Used for debugging
 */
void extdetail_launch_detect_handler(eventid_t id) {
    (void)id;
    int     pinval;
    // BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;

    pinval = palReadPad(GPIOD, 11);

    if(pinval == PAL_LOW) {
    	launch_detected = false;
    } else {
    	launch_detected = true;
    }

    data_udp_tx_launch_det(&launch_detected) ;

   // chprintf(chp, "\r\nLaunch Detect: %u\r\n", launch_detected);
}

//static void green_led_off(void *arg) {
//	(void)arg;
//	palSetPad(GPIOC, GPIOC_LED);
//}

/*!
 * WKUP button handler
 *
 * Used for debugging
 */
void extdetail_WKUP_button_handler(eventid_t id) {
    (void)id;
     ++extdetail_wkup_button_pressed;
   // BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
   // chprintf(chp, "\r\nWKUP btn. eventid: %d\r\n", id);

}

/*! Triggered when the WKUP button is pressed or released. The LED is set to ON.
 *
 * Challenge: Add de-bouncing
 */
void extdetail_wkup_btn(EXTDriver *extp, expchannel_t channel) {
	//static VirtualTimer vt4;

	(void)extp;
	(void)channel;

	//palClearPad(GPIOC, GPIOC_LED);
	chSysLockFromIsr();
	chEvtBroadcastI(&extdetail_wkup_event);

//	if (chVTIsArmedI(&vt4))
//		chVTResetI(&vt4);

	/* LED4 set to OFF after 500mS.*/
	//chVTSetI(&vt4, MS2ST(500), green_led_off, NULL);
	chSysUnlockFromIsr();
}

/*! Triggered when the pin changes state (both edges)
 *
 * Challenge: Add de-bouncing
 */
void extdetail_launch_detect(EXTDriver *extp, expchannel_t channel) {

    (void)extp;
    (void)channel;

    chSysLockFromIsr();
    chEvtBroadcastI(&extdetail_launch_detect_event);

    chSysUnlockFromIsr();
}

//! @}

