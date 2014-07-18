/*! \file iwdg_lld.h
 *
 */


#ifndef _IWDG_LLD_H
#define _IWDG_LLD_H

/*!
 * \addtogroup iwdg_lld
 * @{
 */

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined (STM32F4XX)
 #error "Please select first the target STM32F4XX device used in your application (in stm32f4xx.h file)"
#endif

void iwdgStart(void) ;

#ifdef __cplusplus
}
#endif

//! @}

#endif
