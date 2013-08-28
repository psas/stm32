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


/*! \warning PERIOD setting for PWM is uint16_t (0-65535)
 *
 *    Do not overflow this value! You may have to turn down
 *    INIT_PWM_FREQ to obtain pulsewidths you need.
 *
#define         INIT_PWM_FREQ                  3000000
#define         INIT_PWM_PERIOD_TICS           ((pwmcnt_t)  60000)
#define         INIT_PWM_PULSE_WIDTH_TICS      ((pwmcnt_t)  4500)
 */

/*

 */
//
//#define         INIT_PWM_FREQ                  25000000
//#define         INIT_PWM_PERIOD_TICS           ((pwmcnt_t) 1000)
//#define         INIT_PWM_PULSE_WIDTH_TICS      ((pwmcnt_t) 500)

#define         INIT_PWM_FREQ                  28000000
#define         INIT_PWM_PERIOD_TICS           ((pwmcnt_t) 2)
#define         INIT_PWM_PULSE_WIDTH_TICS      ((pwmcnt_t) 1)


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
