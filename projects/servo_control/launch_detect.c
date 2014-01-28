#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "data_udp.h"
#include "usbdetail.h"

#include "launch_detect.h"



// must be declared here in order to configure the external interrupt below
void launch_detect_isr(EXTDriver *extp, expchannel_t channel);

// setup rising and falling external interrupts on PD11 to trigger the launch_detect_isr
static const EXTConfig extcfg = {
    {
        {EXT_CH_MODE_DISABLED, NULL}, // 0
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
 * Global State Variables
 * ====================== ******************************************************
 */

// Statically initialized to false, can be changed by:
//   1) the launch_detect_init function
//   2) the launch_detect_handler
// if either of them thinks that we have launched based on the state of pin D11,
// after ghetto-debouncing by reading the pin twice with a short sleep between
// the reads.
bool launch_detected = false;

static WORKING_AREA(wa_launch_detect_dispatcher, 1024);

static EventSource launch_detect_event;
static EventSource wakeup_event;


// other private function declarations

static msg_t launch_detect_dispatcher(void *unused);
static void launch_detect_handler(eventid_t _);



/*
 * Public Function Definitons
 * ========================== **************************************************
 */

bool get_launch_detected(void) {
    return launch_detected;
}

void launch_detect_init() {
    chEvtInit(&wakeup_event);
    chEvtInit(&launch_detect_event);

    // activate EXT peripheral so we can get external interrupts
    extStart(&EXTD1, &extcfg);

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

    chThdCreateStatic( wa_launch_detect_dispatcher
                     , sizeof(wa_launch_detect_dispatcher)
                     , NORMALPRIO
                     , launch_detect_dispatcher
                     , NULL
                     );
}



/*
 * Other Function Definitions
 * ========================== **************************************************
 */

/*
 * Launch Detect Thread
 *
 * This thread just waits for the external interrupt to broadcast the
 * launch_detect_event, and dispatches it to the launch_detect_handler.
 */
static msg_t launch_detect_dispatcher(void *unused) {
    // FIXME: use "unused" attribute
    (void)unused;

    struct EventListener launch_event_listener;
    static const evhandler_t launch_event_handlers[] = {
        launch_detect_handler
    };

    chRegSetThreadName("launch_detect");
    launch_detect_init();

    chEvtRegister(&launch_detect_event, &launch_event_listener, 0);

    while (TRUE) {
        chEvtDispatch(launch_event_handlers, chEvtWaitOne(ALL_EVENTS));
    }

    return -1;
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

    data_udp_tx_launch_det(&launch_detected);

#ifdef DEBUG_LAUNCH_DETECT
    BaseSequentialStream *usbserial = (BaseSequentialStream *)&SDU_PSAS;
    chprintf(usbserial, "\r\nLaunch Detect: %u\r\n", launch_detected);
#endif
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
