/*! \file pwmdetail.c
 *
 */

#include "ch.h"
#include "hal.h"

#include "pwm_lld.h"



PWMConfig pwmcfg = {
		1000000, /* 10Khz PWM clock frequency */
		20000,   /* PWM period 1 second */
		NULL,    /* No callback */
		/* Only channel 4 enabled */
		{
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_DISABLED, NULL},
				{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		},
		0
};
