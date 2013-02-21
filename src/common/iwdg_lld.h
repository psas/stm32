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

typedef enum {
	IWDG_PS_DIV4 = 0,
	IWDG_PS_DIV8,
	IWDG_PS_DIV16,
	IWDG_PS_DIV32,
	IWDG_PS_DIV64,
	IWDG_PS_DIV128,
	IWDG_PS_DIV256
} iwdg_ps_val;


void iwdg_lld_init(void) ;
void iwdg_lld_reload(void) ;
void iwdg_lld_set_prescale(iwdg_ps_val s) ;
void iwdg_lld_set_reloadval(uint16_t r) ;

#ifdef __cplusplus
}
#endif

//! @}

#endif
