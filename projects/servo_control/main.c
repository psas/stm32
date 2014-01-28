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
#include "cmddetail.h"
#include "launch_detect.h"
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

#define DEBUG_PWM 1

static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"pwm", cmd_pwmlims},
    {NULL, NULL}
};

static const ShellConfig shell_cfg = {
    (BaseSequentialStream *)&SDU_PSAS,
    commands
};



/*
 * Global Variables
 * ================ ************************************************************
 */

static WORKING_AREA(wa_led_blinker, 128);
static WORKING_AREA(wa_watchdog_keeper, 128);

#if DEBUG_PWM
static WORKING_AREA(wa_pwm_tester, 512);

int pwm_lo = 1300;
int pwm_hi = 1700;
#endif



/*
 * Threads
 * ======= *********************************************************************
 */


/*
 * LED Blinker Thread
 *
 * This thread blinks LEDs so we can tell that we have not halted as long as the
 * LEDs are still blinking.
 */
static msg_t led_blinker(void *_) {
    (void)_;

    chRegSetThreadName("blinker");

    while (TRUE) {
        palTogglePad(GPIOC, GPIOC_LED);
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
 * Test PWM Thread
 *
 * Steps up and then down through servo positions.
 */
#if DEBUG_PWM
static msg_t pwm_tester(void *_) {
    (void)_;

    chRegSetThreadName("pwmtest");

    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    chThdSleepMilliseconds(1000);

    while(1) {
        int32_t pulse = 0;

        for (pulse = pwm_lo; pulse <= pwm_hi; pulse += 50) {
            pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
            chprintf(chp, "%d\r\n", pulse);
            chThdSleepMilliseconds(50);
        }

        chprintf(chp, "rest\r\n");
        chThdSleepMilliseconds(5000);

        for (pulse = pwm_hi; pulse >= pwm_lo; pulse -= 50) {
        	pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
            chprintf(chp, "%d\r\n", pulse);
        	chThdSleepMilliseconds(50);
        }

        chprintf(chp, "rest\r\n");
        chThdSleepMilliseconds(5000);

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
}


int main(void) {
    Thread *shelltp = NULL;
    struct lwipthread_opts ip_opts;

    /* initialize HAL */
    halInit();
    chSysInit();

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

    // initialize wakeup & launch detect events
    launch_detect_init();

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

    chThdCreateStatic( wa_data_udp_rx_thread
                     , sizeof(wa_data_udp_rx_thread)
                     , NORMALPRIO
                     , data_udp_rx_thread
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

    while (TRUE) {
        if (!shelltp && (SDU_PSAS.config->usbp->state == USB_ACTIVE)) {
            // create the shell thread if it needs to be and our USB connection is up
            shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, NORMALPRIO);
        } else if (chThdTerminated(shelltp)) {
            chThdRelease(shelltp); /* Recovers memory of the previous shell.   */
            shelltp = NULL;        /* Triggers spawning of a new shell.        */
        }

        chThdSleepMilliseconds(100);
    }
}
