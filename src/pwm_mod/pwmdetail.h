/*! \file pwmdetail.h
 *
 */


#ifndef _PWMDETAIL_H
#define _PWMDETAIL_H

#include "ch.h"
#include "hal.h"

#define  INIT_PWM_FREQ        1000000
#define  INIT_PWM_WIDTH_TICS  20
#define  INIT_PWM_PERIOD      400

extern PWMConfig         pwmcfg ;
extern PWMConfig         pwmcfg_led ;   /* play with pulsing an led */


#endif
