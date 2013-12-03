/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth. An interrupt state determines
 * launch detect.
 */

/*
 * Includes
 * ======== ********************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <lwip/ip_addr.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"
#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"
#include "pwm.h"
#include "pwm_config.h"
#include "device_net.h"
#include "fc_net.h"



/*
 * Constant Definitions
 * ==================== ********************************************************
 */

//#define DEBUG_PWM 0

static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SDU_PSAS,
    commands
};



/*
 * Global Variables
 * ================ ************************************************************
 */

static WORKING_AREA(wa_launch_detect_dispatcher, 1024);
static WORKING_AREA(wa_led_blinker, 128);
static WORKING_AREA(wa_watchdog_keeper, 128);

#ifdef DEBUG_PWM
static WORKING_AREA(wa_pwm_tester, 512);
#endif



/*
 * Threads
 * ======= *********************************************************************
 */

/*
 * Launch Detect Thread
 *
 * This thread just waits for the external interrupt to broadcast the
 * extdetail_launch_detect_event, and dispatches it to the
 * extdetail_launch_detect_handler.
 */
static msg_t launch_detect_dispatcher(void *_) {
    (void)_;

    struct EventListener launch_event_listener;
    static const evhandler_t launch_event_handlers[] = {
        extdetail_launch_detect_handler
    };

    chRegSetThreadName("launch_detect");
    launch_detect_init();
    chEvtRegister(&extdetail_launch_detect_event, &launch_event_listener, 0);

    while (TRUE) {
        chEvtDispatch(launch_event_handlers, chEvtWaitOne(EVENT_MASK(0)));
    }
    return -1;
}


/*
 * LED Blinker Thread
 *
 * This thread blinks LEDs so we can tell that we have not halted as long as the
 * LEDs are still blinking.
 */
static msg_t led_blinker(void *_) {
    (void)_;

    chRegSetThreadName("blinker");
    palClearPad(GPIOF, GPIOF_GREEN_LED);
    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_BLUE_LED);

    while (TRUE) {
        palTogglePad(GPIOC, GPIOC_LED);
        palTogglePad(GPIOF, GPIOF_GREEN_LED);
        chThdSleepMilliseconds(500);
    }
    return -1;
}


/*
 * Watchdog Thread
 *
 * Keep the watchdog at bay - we power cycle if this thread doesn't run.
 */
static msg_t watchdog_keeper(void *_) {
    (void)_;

    chRegSetThreadName("iwatchdog");

    while (TRUE) {
        iwdg_lld_reload();
        chThdSleepMilliseconds(250);
    }
    return -1;
}


/*
 * Test PWM Sequence Thread
 *
 * Runs through max left, neutral, and max right servo positions
 */
#if DEBUG_PWM
static msg_t pwm_tester(void *arg) {
    (void)arg;
    chRegSetThreadName("pwmtest");

    //    uint32_t i = 0;
     BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    //
    chThdSleepMilliseconds(1000);

    chprintf(chp, "%d %d %d\n", pwm_us_to_ticks(1050), pwm_us_to_ticks(1500), pwm_us_to_ticks(1090));

    while(1) {
        uint32_t pulse = 0;

        //     chprintf(chp, ".");
        //        pwm_set_pulse_width_ticks(pwm_us_to_ticks(333));
        //        chThdSleepMilliseconds(200);
        //
        //        pwm_set_pulse_width_ticks(pwm_us_to_ticks(1050));
        //        chThdSleepMilliseconds(200);
        //
        //        pwm_set_pulse_width_ticks(pwm_us_to_ticks(1900));
        //        chThdSleepMilliseconds(200);
        //
        //        pwmDisableChannel(&PWMD4, 3);
        //chThdSleepMilliseconds(250);
        for (pulse = 1050; pulse <= 1900; pulse += 10) {
            pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
            chThdSleepMilliseconds(50);
        }
        pwmDisableChannel(&PWMD4, 3);
        chThdSleepMilliseconds(2500);

        for (pulse = pulse; pulse > 1050; pulse -= 10) {
        	pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
        	chThdSleepMilliseconds(50);
        }
//

    }
    return -1;
}
#endif



/*
 * Hardware Init & Main
 * ==================== ********************************************************
 */

static void led_init(void) {
    palSetPad(GPIOC, GPIOC_LED);
    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_GREEN_LED);
    palClearPad(GPIOF, GPIOF_BLUE_LED);

    int i = 0;
    for(i=0; i<5; ++i) {
        palSetPad(GPIOF, GPIOF_RED_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_RED_LED);
        palSetPad(GPIOF, GPIOF_GREEN_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_GREEN_LED);
        palSetPad(GPIOF, GPIOF_BLUE_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_BLUE_LED);
    }
}


int main(void) {
    static Thread          *shelltp = NULL;
    struct EventListener   wakeup_listener;
    struct lwipthread_opts ip_opts;
    static const evhandler_t wakeup_handlers[] = {
        extdetail_WKUP_button_handler
    };

    /* initialize HAL */
    halInit();
    chSysInit();

    // initialize wakeup & launch detect events
    extdetail_init();

    // run through colored lights, for funzies i guess
    led_init();

    // start serial-over-USB
    sduObjectInit(&SDU_PSAS);
    sduStart(&SDU_PSAS, &serusbcfg);
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // start the command shell
    shellInit();

    // start the watchdog timer
    iwdg_begin();

    // start the serial driver (TODO: why?)
    sdStart(&SD6, NULL);

    // why does this exist?
    chThdSleepMilliseconds(300);

    // activate EXT peripheral so we can get external interrupts
    extStart(&EXTD1, &extcfg);

    // activate PWM output
    pwm_start();

    // configure IP
    static uint8_t roll_ctl_mac_address[6] = ROLL_CTL_MAC_ADDRESS;
    struct ip_addr ip, gateway, netmask;
    ROLL_CTL_IP_ADDR(&ip);
    ROLL_CTL_GATEWAY(&gateway);
    ROLL_CTL_NETMASK(&netmask);
    ip_opts.macaddress = roll_ctl_mac_address;
    ip_opts.address    = ip.addr;
    ip_opts.netmask    = netmask.addr;
    ip_opts.gateway    = gateway.addr;

    // start threads
    chThdCreateStatic( wa_led_blinker
                     , sizeof(wa_led_blinker)
                     , NORMALPRIO
                     , led_blinker
                     , NULL
                     );

    chThdCreateStatic( wa_lwip_thread
                     , sizeof(wa_lwip_thread)
                     , NORMALPRIO + 2
                     , lwip_thread
                     , &ip_opts
                     );

    chThdCreateStatic( wa_data_udp_receive_thread
                     , sizeof(wa_data_udp_receive_thread)
                     , NORMALPRIO
                     , data_udp_receive_thread
                     , NULL
                     );

    chThdCreateStatic( wa_launch_detect_dispatcher
                     , sizeof(wa_launch_detect_dispatcher)
                     , NORMALPRIO
                     , launch_detect_dispatcher
                     , NULL
                     );

    chThdCreateStatic( wa_watchdog_keeper
                     , sizeof(wa_watchdog_keeper)
                     , NORMALPRIO
                     , watchdog_keeper
                     , NULL
                     );

#if DEBUG_PWM
    chThdCreateStatic( wa_pwm_tester
                     , sizeof(wa_pwm_tester)
                     , NORMALPRIO
                     , pwm_tester
                     , NULL
                     );
#endif

    chEvtRegister(&extdetail_wkup_event, &wakeup_listener, 0);

    while (TRUE) {
        if (!shelltp && (SDU_PSAS.config->usbp->state == USB_ACTIVE)) {
            // create the shell thread if it needs to be and our USB connection is up
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        } else if (chThdTerminated(shelltp)) {
            chThdRelease(shelltp); /* Recovers memory of the previous shell.   */
            shelltp = NULL;        /* Triggers spawning of a new shell.        */
        }

        chEvtDispatch(wakeup_handlers, chEvtWaitOneTimeout((eventmask_t)0, MS2ST(500)));
    }
}
