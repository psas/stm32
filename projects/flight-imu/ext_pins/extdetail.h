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

void extdetail_init(void) ;
void extdetail_wkup_btn(EXTDriver *extp, expchannel_t channel) ;
void extdetail_adis_dio1(EXTDriver *extp, expchannel_t channel) ;
void extdetail_mpu9150_int(EXTDriver *extp, expchannel_t channel) ;
void extdetail_mpl3115a2_int_1(EXTDriver *extp, expchannel_t channel) ;


#ifdef __cplusplus
}
#endif



//! @}


#endif

