/*! \file extdetail.h
 *
 */

#ifndef _EXTDETAIL_H
#define _EXTDETAIL_H

/*!
 * \addtogroup extdetail
 * @{
 */
#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const     EXTConfig       extcfg;
extern           EventSource     wkup_event;
extern           EventSource     spi1_event;

void green_led_off(void *arg);
void extcb1(EXTDriver *extp, expchannel_t channel) ;

#ifdef __cplusplus
}
#endif



//! @}


#endif

