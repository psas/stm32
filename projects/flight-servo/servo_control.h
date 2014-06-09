#ifndef _PWM_CONFIG_H
#define _PWM_CONFIG_H

#include <stdint.h>

#include "ch.h"
#include "hal.h"

/*
 * \warning PERIOD setting for PWM is uint16_t (0-65535)
 *
 * Do not overflow this value! You may have to turn down
 * INIT_PWM_FREQ to obtain pulsewidths you need. See Hitec
 * 20mS example settings.
 * Testing only:
 * Hitec 20mS period (50hZ) servos (Models HS-81 and HS-45HB)
 *  Pulse widths range from 900 uS to 2100 uS

#define         INIT_PWM_FREQ                  3000000
#define         INIT_PWM_PERIOD_TICKS           ((pwmcnt_t)  60000)
#define         INIT_PWM_PULSE_WIDTH_TICKS      ((pwmcnt_t)  4500)
*/

/*
 * Rocket servo
 * 3.3mS period (300hZ)
 * JS DS8717 Servo
 * http://www.servodatabase.com/servo/jr/ds8717
 *
 * Pulse widths range from 1100 uS to 1900 uS
 * Hard stops on mechanism define this range. Center 1500uS pw.
 *
 */

/*
 * Servo PWM Constants
 * ===================
 */

#define PWM_CENTER 1500

// Absolute position limits, in microseconds (us).
// Verified empirically by DLP, K, and Dave 3/25/14
#define PWM_LO 1100
#define PWM_HI 1900

// Slew rate limit, in microseconds/millisecond. This limit corresponds to being
// able to go from center to far mechanical limit in 100 ms.
#define PWM_MAX_RATE 5

// Maximum allowable oscillation frequency. We prevent the servo from changing
// direction any faster than this.
#define PWM_MAX_OSCILLATION_FREQ 50
#define PWM_MIN_DIRECTION_CHANGE_PERIOD (1000 / PWM_MAX_OSCILLATION_FREQ)


/*
 * Servo Command Mailboxen Parameters
 * ================================== ******************************************
 */

/*
 * Data Structures
 * =============== *************************************************************
 */

typedef struct PositionCommand {
  uint16_t position;
} PositionCommand;

typedef struct PositionDelta {
  uint16_t delta;
} PositionDelta;


/*
 * Function Declarations
 * ===================== *******************************************************
 */

void pwm_start(void);
void pwm_set_pulse_width_ticks(uint32_t width_ticks);
uint32_t pwm_get_period_ms(void);
pwmcnt_t pwm_us_to_ticks(uint32_t us) ;

#endif
