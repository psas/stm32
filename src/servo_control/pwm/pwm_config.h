/*! \file pwm_config.h
 *
 */

#ifndef _PWM_CONFIG_H
#define _PWM_CONFIG_H

/*!
 * \addtogroup pwm_config
 * @{
 */

/*!
 * Try to standardize units on 'ticks'
 * One 'tick' is one PWM clock.
 */

#include <stdint.h>

#include "ch.h"
#include "hal.h"
#include "math.h"

#include "pwm_lld.h"

#define         INIT_PWM_FREQ                  6000000
#define         INIT_PWM_PERIOD_TICS           ((pwmcnt_t) 20000)
#define         INIT_PWM_PULSE_WIDTH_TICS      ((pwmcnt_t) 10000)

#ifdef __cplusplus
extern "C" {
#endif

void            pwm_start(void);
void            pwm_set_pulse_width_ticks(uint32_t width_ticks);
void            pwm_set_period(uint32_t per_ticks);

uint32_t        pwm_get_PWM_freq_hz(void);
uint32_t        pwm_get_period_ms(void);

pwmcnt_t        pwm_get_pulse_width(void);
pwmcnt_t        pwm_us_to_ticks(uint32_t us) ;

#ifdef __cplusplus
}
#endif
//! @}

#endif
