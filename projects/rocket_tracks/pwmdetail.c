/*! \file pwmdetail.c
 *
 */

#include "ch.h"
#include "hal.h"

#include "pwm_lld.h"



PWMConfig pwmcfg = {
		1000000, /* 1Mhz PWM clock frequency */
		1000,   /* PWM period 1 millisecond */
		NULL,    /* No callback */
		/* All 4 channels enabled */
		{
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		},
		0
};
