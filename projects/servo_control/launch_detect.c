#include <string.h>

#include "ch.h"
#include "hal.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "device_net.h"
#include "launch_detect.h"

#ifdef DEBUG_LAUNCH_DETECT
#include "chprintf.h"
#include "usbdetail.h"
#endif

#define UNUSED __attribute__((unused))

#define LD_PORT  GPIOD    // launch detect pin port
#define LD_PIN   11       // launch detect pin
#define LD_LEVEL PAL_HIGH // active level (i.e. active high or low?)

static bool launch_detected; // Changed in isr, launch_detect_init

static EventSource launch_detect_event;

static int ld_socket; // Used in handler, set in launch_detect_init

/*
 * Triggered by a level change on the launch detect pin.
 * launch_detect_event is handled by the handler below.
 */
// FIXME: pin debouncing
static void isr(EXTDriver *e UNUSED, expchannel_t c UNUSED) {
    chSysLockFromIsr();
    if (palReadPad(LD_PORT, LD_PIN) == LD_LEVEL) {
        launch_detected = true;
    } else {
        launch_detected = false;
    }
    chEvtBroadcastI(&launch_detect_event);
    chSysUnlockFromIsr();
}

// Sends the launch state to the flight computer
static void handler(eventid_t u UNUSED) {
    write(ld_socket, &launch_detected, 1);

#ifdef DEBUG_LAUNCH_DETECT
    BaseSequentialStream *usbserial = getActiveUsbSerialStream();
    chprintf(usbserial, "\r\nLaunch Detect: %u\r\n", launch_detected);
#endif
}

/*
 * Launch Detect Dispatcher Thread
 *
 * Waits for the external interrupt to broadcast launch_detect_event,
 * and dispatches it to the launch detect handler.
 */
static WORKING_AREA(wa_dispatcher, 512);

static msg_t dispatcher(void *u UNUSED) {
    chRegSetThreadName("launch_detect");

    struct EventListener launch_event_listener;
    static const evhandler_t launch_event_handlers[] = {
        handler
    };
    chEvtRegister(&launch_detect_event, &launch_event_listener, 0);

    while (TRUE) {
        chEvtDispatch(launch_event_handlers, chEvtWaitOne(ALL_EVENTS));
    }

    return -1;
}

/*
 * Public Function Definitons
 * ========================== **************************************************
 */

bool get_launch_detected(void) {
    return launch_detected;
}

void launch_detect_init() {
    // setup rising and falling external interrupts on PD11 to trigger the launch_detect_isr
    // should be static because extStart doesn't deep copy
    static const EXTConfig extcfg =
    {
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
            {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOD, isr},   // 11
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
    // activate EXT peripheral so we can get external interrupts
    extStart(&EXTD1, &extcfg);

    // initialize pin level
    if (palReadPad(LD_PORT, LD_PIN) == LD_LEVEL) {
        launch_detected = true;
    }else{
        launch_detected = false;
    }

    //FIXME: use common code networking
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET,
    addr.sin_port = htons(ROLL_CTL_TX_PORT);
    inet_aton(ROLL_CTL_IP_ADDR_STRING , &addr.sin_addr);

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(36000);
    inet_aton("10.0.0.10", &dest_addr.sin_addr);

    ld_socket = socket(AF_INET,  SOCK_DGRAM, 0);
    bind(ld_socket, (struct sockaddr*)&addr, sizeof(addr));
    connect(ld_socket, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

    handler(0); //sends initial launch status

    chEvtInit(&launch_detect_event);

    chThdCreateStatic( wa_dispatcher
                     , sizeof(wa_dispatcher)
                     , NORMALPRIO
                     , dispatcher
                     , NULL
                     );
}

