/*! \file pwmdetail.h
 *
 */


#ifndef _PWMDETAIL_H
#define _PWMDETAIL_H

/*!
 * \addtogroup pwmdetail
 * @{
 */

#include "ch.h"
#include "hal.h"

#define  INIT_PWM_FREQ        1000000
#define  INIT_PWM_WIDTH_TICS  20
#define  INIT_PWM_PERIOD      400


#ifdef __cplusplus
extern "C" {
#endif


extern PWMConfig         pwmcfg ;
extern PWMConfig         pwmcfg_led ;   /* play with pulsing an led */



#ifdef __cplusplus
}
#endif


//! @}


#endif
