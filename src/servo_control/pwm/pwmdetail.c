/*! \file pwmdetail.c
 *
 */

/*!
 * \defgroup pwmdetail PWM Utilities
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "pwm_lld.h"
#include "pwmdetail.h"


PWMConfig pwmcfg = {
		1000000,            /* 1 Mhz PWM clock frequency, 1 uS per tick */
		1000,               /* PWM period 1 mS */
		NULL,               /* No callback */
		/* Only channel 4 enabled */
		{
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		},
		0
};

PWMConfig pwmcfg_led = {
		INIT_PWM_FREQ,
		INIT_PWM_PERIOD,
		NULL,               /* No callback */
		/* Only channel 4 enabled */
		{
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		},
		0
};

//! @}
