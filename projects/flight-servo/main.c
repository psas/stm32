/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth.
 */

// ChibiOS
#include "ch.h"
#include "hal.h"

// PSAS common
#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_led.h"

// servo_control
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

    while (true) {
        chThdSleep(TIME_INFINITE);
    }
}
