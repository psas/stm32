/*
 * Manage the pwm for the servo. We are using the JS DS8717 Servo. Details here:
 * http://www.servodatabase.com/servo/jr/ds8717
 *
 * The pulse widths on our servo range from 333 us to 1520 us.
 * The usable roll control range on our servo is between 1000 us to 2000 us,
 * with the center sensibly at 1500 us.
 * FIXME: is it really? I found *three* different definitions of the servo
 * range:
 *   1). Comments near the top of this file said "from 333 us to 1520 us" (right
 *       after a link to the servo's datasheet).
 *   2). Comments near the top of this file's header said that "for testing
 *       only" the range is "from 900 us to 2100 us".
 *   3). Comments also in the header, but after those referenced in 2 define the
 *       range to be 1000 us to 2000us, and claims these are enforced by
 *       mechanical stops, so that's what I took.
 *
 * Attempt to standardize units on 'ticks' of PWM module. One PWM clock is one
 * 'tick'.
 */

// stdlib
#include <stdint.h>
#include <string.h>

// ChibiOs
#include "ch.h"
#include "hal.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "chprintf.h"

// PSAS
#include "usbdetail.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "psas_packet.h"
#include "servo_control.h"

#define UNUSED __attribute__((unused))

/*
 * Constant Definitions
 * ==================== ********************************************************
 */

#define         INIT_PWM_FREQ                  6000000
#define         INIT_PWM_PERIOD_TICKS           ((pwmcnt_t) 20000)
#define         INIT_PWM_PULSE_WIDTH_TICKS      ((pwmcnt_t) 9000)

// This enum corresponds to the PWM bisable bit in the message received from the
// flight computer. Since it's a _disable_ bit, when it is zero then the PWM is
// enabled.
enum {
    PWM_ENABLE = 0,
    PWM_DISABLE
};


void pwm_set_pulse_width_ticks(uint32_t ticks){
    pwmEnableChannel(&PWMD4, 3, ticks);
}

/*
 * This is a conversion function to return the period in ms.
 */
uint32_t pwm_get_period_ms() {
    return (uint32_t) (PWMD4.tim->CCR[3]/PWMD4.period * 1000) ;
}

/*
 * Convert from microseconds to PWM ticks (1 second = PWM_FREQ ticks)
 */
pwmcnt_t pwm_us_to_ticks(uint32_t us) {
    //fixme: numerical stability of doubles
    double   ticks_per_us_quot;
    uint32_t ticks_per_us;

    ticks_per_us_quot = INIT_PWM_FREQ/1e6;
    ticks_per_us      = (pwmcnt_t) ticks_per_us_quot;

    return (pwmcnt_t) (us * ticks_per_us);
}


/*
 * PWM control thread - sets up a listen socket and then sets the pwm width
 * according to the data received.
 */
WORKING_AREA(wa_pwm_thread, 1024);
msg_t pwm_thread(void * u UNUSED) {
    chRegSetThreadName("pwm");

//    BaseSequentialStream * chp = getUsbStream();

    RCOutput       rc_packet;
    char data[sizeof(RCOutput)];

    const int pwm_lo = 1200;
    const int pwm_hi = 1800;

    int socket = get_udp_socket(ROLL_ADDR);
    if(socket < 0){
        return -1;
    }

//    chprintf(chp, "starting \r\n");
    pwmcnt_t ticks = INIT_PWM_PULSE_WIDTH_TICKS;

    uint16_t prev_width =1500;
    while(TRUE){
        //fixme: throw away anything left
        read(socket, data, sizeof(data));
        memcpy(&rc_packet, data, sizeof(RCOutput));
//        chThdSleepMilliseconds(400);
//        rc_packet.u16ServoPulseWidthBin14 = 1500;
//        rc_packet.u8ServoDisableFlag = FALSE;

//        chprintf(chp, "Got data :%d\r\n",rc_packet.u16ServoPulseWidthBin14 );

//        if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
            uint16_t width = rc_packet.u16ServoPulseWidthBin14;

            if(width > prev_width + 3){
                width = prev_width + 3;
            }else if(width < prev_width - 3){
                width = prev_width - 3;
            }
            prev_width = width;

            if(width < pwm_lo){
                ticks =  pwm_us_to_ticks(pwm_lo);
            } else if (width > pwm_hi){
                ticks =  pwm_us_to_ticks(pwm_hi);
            } else {
                ticks = pwm_us_to_ticks(width);
            }

            pwmEnableChannel(&PWMD4, 3, ticks);
//        } else {
//            pwmDisableChannel(&PWMD4, 3);
//        }

    }

    return -1;
}

void pwm_start() {
    // Static because pwmStart doesn't deep copy PWMConfigs
    static PWMConfig pwmcfg = {
        .frequency = INIT_PWM_FREQ, /* 6Mhz PWM clock frequency; (1/f) = 'ticks' => 167ns/tick */
        .period = INIT_PWM_PERIOD_TICKS, /* PWM period_ticks in ticks;
                                 (T) in ticks * us/tick = 20000 ticks * 167nS/tick = 3.34mS period =>  300 Hz  */
        .callback = NULL,                 /* No callback */
        .channels = {
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},   /* Only channel 4 enabled, PD15 (Physical pin 18) */
        },
        .cr2 = 0
    };


    palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
    pwmStart(&PWMD4, &pwmcfg);
    pwmEnableChannel(&PWMD4, 3, INIT_PWM_PULSE_WIDTH_TICKS);
//    pwmDisableChannel(&PWMD4, 3);

    chThdCreateStatic( wa_pwm_thread
                     , sizeof(wa_pwm_thread)
                     , NORMALPRIO
                     , pwm_thread
                     , NULL
                     );
}
