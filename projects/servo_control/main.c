/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth. An interrupt state determines
 * launch detect.
 */

// ChibiOS
#include "ch.h"
#include "hal.h"
#include "lwip/ip_addr.h"
#include "lwipopts.h"
#include "lwipthread.h"

// PSAS common
#include "net_addrs.h"

// servo_control
#include "launch_detect.h"
#include "servo_control.h"

#define DEBUG_PWM 1

#if DEBUG_PWM
#include "debug_pwm.h"
#endif

#define UNUSED __attribute__((unused))
/*
 * LED Blinker Thread
 *
 * This thread blinks LEDs so we can tell that we have not halted as long as the
 * LEDs are still blinking.
 */

#define GPIOF_GREEN_LED             2
#define GPIOF_RED_LED               3
#define GPIOF_BLUE_LED              14

static WORKING_AREA(wa_led, 128);

static msg_t led(void * u UNUSED) {
    chRegSetThreadName("blinker");

    while (TRUE) {
        palTogglePad(GPIOC, GPIOC_LED);
        chThdSleepMilliseconds(500);
    }
    return -1;
}

static void led_init(void) {
    palSetPad(GPIOC, GPIOC_LED);
    chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO, led, NULL);
}

void main(void) {
    /* initialize HAL */
    halInit();
    chSysInit();
    led_init();


    chThdCreateStatic( wa_lwip_thread
                     , sizeof(wa_lwip_thread)
                     , NORMALPRIO + 2
                     , lwip_thread
                     , ROLL_LWIP
                     );

    // activate PWM output
    pwm_start();
#if DEBUG_PWM
    // Starts usb-serial terminal, pwm_tester thread
    debug_pwm_start();
#endif
    // initialize launch detection subsystem
    launch_detect_init();
    while (true) {
        chThdSleep(TIME_INFINITE);
    }
}
