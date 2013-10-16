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

extern           EventSource     extdetail_wkup_event;

extern const     EXTConfig       extcfg;

void extdetail_WKUP_button_handler(eventid_t id) ;
void extdetail_init(void) ;
void extdetail_wkup_btn(EXTDriver *extp, expchannel_t channel) ;

#ifdef __cplusplus
}
#endif



//! @}


#endif

