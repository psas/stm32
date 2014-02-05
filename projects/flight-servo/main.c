/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth. An interrupt state determines
 * launch detect.
 */

// ChibiOS
#include "ch.h"
#include "hal.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwipopts.h"
#include "lwipthread.h"
#include "chprintf.h"

// PSAS common
#include "net_addrs.h"
#include "usbdetail.h"
#include "utils_led.h"

// servo_control
#include "launch_detect.h"
#include "servo_control.h"

#define DEBUG_PWM 1

#if DEBUG_PWM
#include "debug_pwm.h"
#endif


void main(void) {
    /* initialize HAL */
    halInit();
    chSysInit();
    led_init(&e407_led_cfg);

    chThdCreateStatic( wa_lwip_thread
                     , sizeof(wa_lwip_thread)
                     , NORMALPRIO + 2
                     , lwip_thread
                     , ROLL_LWIP
                     );

    // activate PWM output
    pwm_start();
#if DEBUG_PWM
    debug_pwm_start();
#endif
    // initialize launch detection subsystem
    launch_detect_init();
    while (true) {
        chThdSleep(TIME_INFINITE);
    }
}
