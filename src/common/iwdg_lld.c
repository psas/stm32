/*! \file   common/iwdg_lld.c
 *  \brief  IWDG (Independent Watch Dog) subsystem low level driver source.
 *
 *  For STM32F4xx
 */

/*!
 * \defgroup iwdg_lld Independent Watch Dog Driver
 *
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "iwdg_lld.h"

/*! \brief Start the independent watchdog timer
 */
void iwdg_lld_init() {
	chSysLock();
	IWDG->KR = (uint16_t)0xCCCC;
	chSysUnlock();
}

/*! \brief Reload the watchdog by writing the key register
 *
 * 'Feed' the watchdog
 */
void iwdg_lld_reload() {
	chSysLock();
	IWDG->KR = (uint16_t)0xAAAA;
	chSysUnlock();
}

/*! \brief Set the IWDG Clock prescale value
 */
void iwdg_lld_set_prescale(iwdg_ps_val s) {

	chSysLock();
	IWDG->KR = (uint16_t)0x5555;
	IWDG->PR = s;
	IWDG->KR = (uint16_t)0xAAAA;
	chSysUnlock();
}

/*! \brief Set the IWDG Clock prescale value
 */
void iwdg_lld_set_reloadval(uint16_t r) {
	chSysLock();
	r        = (r & 0xFFF);
	IWDG->KR = (uint16_t)0x5555;
	IWDG->PR = r;
	IWDG->KR = (uint16_t)0xAAAA;
	chSysUnlock();
}


//! @}

