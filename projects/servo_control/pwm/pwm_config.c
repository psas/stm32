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
#include <stdlib.h>
#include <stdint.h>

// ChibiOs
#include "ch.h"
#include "hal.h"
#include "pwm_lld.h"

// PSAS
#include "pwm_config.h"
#include "chprintf.h"
#include "usbdetail.h"



/*
 * Private Mutable Variables
 * ========================= ***************************************************
 */

static pwmcnt_t pwm_period_ticks      = INIT_PWM_PERIOD_TICKS;
static pwmcnt_t pwm_pulse_width_ticks = INIT_PWM_PULSE_WIDTH_TICKS;

static PWMConfig pwmcfg = {
    INIT_PWM_FREQ,        /* 6Mhz PWM clock frequency; (1/f) = 'ticks' => 167ns/tick */
    INIT_PWM_PERIOD_TICKS, /* PWM period_ticks in ticks;
                             (T) in ticks * us/tick = 20000 ticks * 167nS/tick = 3.34mS period =>  300 Hz  */
    NULL,                 /* No callback */
    {
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},   /* Only channel 4 enabled, PD15 (Physical pin 18) */
    },
    0
};



/*
 * Function Definitions
 * ==================== *******************************************************
 */

void pwm_start() {
    palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
    pwmStart(&PWMD4, &pwmcfg);
    pwmEnableChannel(&PWMD4, 3, INIT_PWM_PULSE_WIDTH_TICKS);
    pwmDisableChannel(&PWMD4, 3);
}


/*
 * This is an immutable accessor function.
 */
uint32_t pwm_get_PWM_freq_hz() {
    return INIT_PWM_FREQ;
}


/*
 * This is an accessor for a file-private mutable variable.
 */
pwmcnt_t pwm_get_pulse_width() {
    return pwm_pulse_width_ticks;
}


/*
 * This is a conversion function to return the period (a file-private mutable
 * variable) in ms.
 */
uint32_t pwm_get_period_ms() {
    return (uint32_t) (pwm_period_ticks/INIT_PWM_FREQ * 1000) ;
}


/*
 * Set the width of the active pulse portion in ticks, where
 * 1 tick = 1 / PWM_FREQ seconds.
 */
void pwm_set_pulse_width_ticks(uint32_t width_ticks) {
    //pwmDisableChannel(&PWMD4, 3); // what's this about? not necessary?
    // is this just a defensive enable?
    pwmEnableChannel(&PWMD4, 3, width_ticks);
    pwm_pulse_width_ticks = width_ticks;
}


/*
 * Set the total PWM period in ticks, where 1 tick = 1 / PWM_FREQ seconds.
 */
void pwm_set_period(uint32_t per_ticks)  {
    pwmChangePeriod(&PWMD4, per_ticks);
    pwm_period_ticks = per_ticks;
}


/*
 * Convert from microseconds to PWM ticks (1 second = PWM_FREQ ticks)
 */
pwmcnt_t pwm_us_to_ticks(uint32_t us) {
    double   ticks_per_us_quot;
    uint32_t ticks_per_us;

    ticks_per_us_quot = INIT_PWM_FREQ/1e6;
    ticks_per_us      = (pwmcnt_t) ticks_per_us_quot;

    return (pwmcnt_t) (us * ticks_per_us);
}
