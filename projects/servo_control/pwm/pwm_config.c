/*! \file pwm_config.c
 *
 * Manage the pwm for the servo
 *
 * Attempt to standardize units on 'ticks' of PWM module
 *   One PWM clock is one 'tick'
 */

/*!
 * \defgroup pwm_config PWM Configuration for Roll Control Module
 * @{
 */
#include <stdlib.h>
#include <stdint.h>
#include "ch.h"
#include "hal.h"
#include "pwm_lld.h"
#include "pwm_config.h"
#include "chprintf.h"
#include "usbdetail.h"

static uint32_t      pwm_freq_hz                = INIT_PWM_FREQ;

static pwmcnt_t      pwm_period_ticks           = INIT_PWM_PERIOD_TICS;
static pwmcnt_t      pwm_pulse_width_ticks      = INIT_PWM_PULSE_WIDTH_TICS;

/*
    JS DS8717 Servo
    http://www.servodatabase.com/servo/jr/ds8717

    Pulse widths range from 333 uS to 1520 uS
 */
static PWMConfig pwmcfg = {
    INIT_PWM_FREQ,             /* 6Mhz PWM clock frequency; (1/f) = 'ticks' => 167ns/tick */
    INIT_PWM_PERIOD_TICS,      /* PWM period_ticks in ticks; (T) in ticks * us/tick = 20000 ticks * 167nS/tick = 3.34mS period =>  300 Hz  */
    NULL,                      /* No callback */
    {
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},   /* Only channel 4 enabled, PD15 */
    },
    0
};

void pwm_start() {
    palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
    pwmStart(&PWMD4, &pwmcfg);
    pwmEnableChannel(&PWMD4, 3, INIT_PWM_PULSE_WIDTH_TICS);
    pwmDisableChannel(&PWMD4, 3);
}

uint32_t pwm_get_PWM_freq_hz() {
    return pwm_freq_hz;
}

pwmcnt_t pwm_get_pulse_width() {
    return pwm_pulse_width_ticks;
}

/*! \brief Return period
 *
 * @return period in mS
 */
uint32_t pwm_get_period_ms() {
    uint32_t per_ms = 0;

    per_ms   = (uint32_t) (pwm_period_ticks/pwm_freq_hz * 1000) ;

    return per_ms;
}

void pwm_set_pulse_width_ticks(uint32_t width_ticks) {
    //pwmDisableChannel(&PWMD4, 3);

    pwmEnableChannel(&PWMD4, 3, width_ticks);

    pwm_pulse_width_ticks   = width_ticks;
}

void pwm_set_period(uint32_t per_ticks)  {
    pwmChangePeriod(&PWMD4, per_ticks);

    pwm_period_ticks = per_ticks;
}

pwmcnt_t pwm_us_to_ticks(uint32_t us) {

    pwmcnt_t tickval           = 0;

    double   ticks_per_us_quot = 0;
    uint32_t ticks_per_us      = 0;

    ticks_per_us_quot          = pwm_freq_hz/1e6;
    ticks_per_us               = (pwmcnt_t) ticks_per_us_quot;

    tickval                    = (pwmcnt_t) (us * ticks_per_us);

    return tickval;
}

//! @}
