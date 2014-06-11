/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth. An external pin
 * determines launch detect.
 */

// ChibiOS
#include "ch.h"
#include "hal.h"

// PSAS common
#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_led.h"

// servo_control
#include "launch_detect.h"
#include "servo_control.h"

void main(void) {
    /* Initialize Chibios */
    halInit();
    chSysInit();

    /* Diagnostic led */
    ledStart(NULL);

    /* Start lwip stack */
    lwipThreadStart(ROLL_LWIP);

    /* activate PWM output */
    pwm_start();

    /* initialize launch detection subsystem */
    launch_detect_init();

    while (true) {
        chThdSleep(TIME_INFINITE);
    }
}
