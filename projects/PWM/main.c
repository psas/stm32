/*
   ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
   2011,2012 Giovanni Di Sirio.

   This file is part of ChibiOS/RT.

   ChibiOS/RT is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   ChibiOS/RT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <stdio.h>

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"

/* Simple PWM example */

/*
 * STM32F40x includes 4 general purpose timers: TIM2, TIM3, TIM4, TIM5
 * TIM3 and TIM4 are 16 bit timers with PWM support.
 *
 * PWMD4 is a HAL defined variable of type PWMDriver - it is associated with
 * TIM4.
 */


#include "ch.h"
#include "hal.h"

static PWMConfig pwmcfg = {
    1000000, /* 10Khz PWM clock frequency */
    20000, /* PWM period 1 second */
    NULL,  /* No callback */
    /* Only channel 4 enabled */
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    },
    0
};


int main(void)
{

    halInit();
    chSysInit();

    /* Enables PWM output (of TIM4, channel 4) on blue LED connected to PD15 */
    palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));

    pwmStart(&PWMD4, &pwmcfg);
    pwmEnableChannel(&PWMD4, 3, 5000);

    while (TRUE) {
        chThdSleepMilliseconds(5000);
    }
}

