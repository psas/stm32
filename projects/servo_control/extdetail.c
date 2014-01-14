#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "data_udp.h"
#include "usbdetail.h"

#include "extdetail.h"



/*
 * Constant Definitions
 * ==================== ********************************************************
 */

const EXTConfig extcfg = {
    {
        {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, wakeup_button_isr}, // 0
        {EXT_CH_MODE_DISABLED, NULL}, // 1
        {EXT_CH_MODE_DISABLED, NULL}, // 2
        {EXT_CH_MODE_DISABLED, NULL}, // 3
        {EXT_CH_MODE_DISABLED, NULL}, // 4
        {EXT_CH_MODE_DISABLED, NULL}, // 5
        {EXT_CH_MODE_DISABLED, NULL}, // 6
        {EXT_CH_MODE_DISABLED, NULL}, // 7
        {EXT_CH_MODE_DISABLED, NULL}, // 8
        {EXT_CH_MODE_DISABLED, NULL}, // 9
        {EXT_CH_MODE_DISABLED, NULL}, // 10
        {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOD, launch_detect_isr},   // 11
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL}, // 14 R PF3 G PF2 B PF14
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



/*
 * Public Global Variables
 * ======================= *****************************************************
 */

bool        launch_detected = false;
EventSource launch_detect_event;
EventSource wakeup_event;



/*
 * Private Global Variables
 * ======================== ****************************************************
 */

static uint16_t wakeup_button_presses = 0;



/*
 * Public Function Definitons
 * ========================== **************************************************
 */

/*
 * This function initializes the events needed to respond to wakeup button
 * presses and launch detect pin level changes.
 */
void extdetail_init() {
    chEvtInit(&wakeup_event);
    chEvtInit(&launch_detect_event);
}


/*
 * This function initializes the launch_detected global to reflect the current
 * state of the launch_detect pin.
 *
 * Open questions: why is this seperate from extdetail_init? could it be called
 * from there?
 */
void launch_detect_init() {
    int pinval1, pinval2;

    // why do we do this?
    launch_detected = false;

    pinval1 = palReadPad(GPIOD, 11);
    chThdSleepMilliseconds(10);
    pinval2 = palReadPad(GPIOD, 11);

    // FIXME: inconsistent with launch_detect_handler
    if ((pinval1 == PAL_LOW) && (pinval2 == PAL_LOW)) {
        launch_detected = true;
    }
}

/*
 * This function handles the launch_detect_event broadcast by the
 * launch_detect_isr. It re-reads the pin value as a form of basic debouncing,
 * and then passes the launch_detected global by reference to
 * data_udp_tx_launch_det, which sends out a launch detect message via ethernet
 * if the global is still true.
 */
void launch_detect_handler(eventid_t _) {
    (void)_;

    int pinval = palReadPad(GPIOD, 11);

    // FIXME: inconsistent with launch_detect_init
    if (pinval == PAL_LOW) {
    	launch_detected = false;
    } else {
    	launch_detected = true;
    }

    // why call this if launch_detect is false? do we log a false positive?
    data_udp_tx_launch_det(&launch_detected);

#ifdef DEBUG_LAUNCH_DETECT
    BaseSequentialStream *usbserial = (BaseSequentialStream *)&SDU_PSAS;
    chprintf(usbserial, "\r\nLaunch Detect: %u\r\n", launch_detected);
#endif
}


/*
 * This function handles the wakeup_event broadcast by the wakeup_button_isr. It
 * pointlessly increments the wakeup_button_presses global variable, which is
 * not read by anything ever.
 *
 * TODO: delete this or figure out why we can't.
 */
void wakeup_button_handler(eventid_t _) {
    (void)_;

    ++wakeup_button_presses;

#ifdef DEBUG_WAKEUP
    BaseSequentialStream *usbserial = (BaseSequentialStream *)&SDU_PSAS;
    chprintf(usbserial, "\r\nWKUP btn. eventid: %d\r\n", _);
#endif
}


/*
 * This ISR is triggered by any level change on the wakeup button's pin. It just
 * broadcasts the wakeup_event, which is handled by the wakeup_button_handler
 * function above.
 */
void wakeup_button_isr(EXTDriver *extp, expchannel_t channel) {
	(void)extp;
	(void)channel;

	chSysLockFromIsr();
	chEvtBroadcastI(&wakeup_event);
	chSysUnlockFromIsr();
}


/*
 * This ISR is triggered by any level change on the launch detect pin. It just
 * broadcasts the launch_detect_event, which is handled by the
 * launch_detect_handler above.
 */
void launch_detect_isr(EXTDriver *extp, expchannel_t channel) {

    (void)extp;
    (void)channel;

    chSysLockFromIsr();
    chEvtBroadcastI(&launch_detect_event);
    chSysUnlockFromIsr();
}
