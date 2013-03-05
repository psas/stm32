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

/*! \brief check reset status and then start iwatchdog
 *
 * Check the CSR register for reset source then start
 * the independent watchdog counter.
 *
 */
void iwdg_begin(void) {
	// was this a reset caused by the iwdg?
	if( (RCC->CSR & RCC_CSR_WDGRSTF) != 0) {
		// \todo Log WDG reset event somewhere.
		RCC->CSR = RCC->CSR | RCC_CSR_RMVF;  // clear the IWDGRSTF
	}
	iwdg_lld_set_prescale(IWDG_PS_DIV32); // This should be about 2 second at 32kHz
	iwdg_lld_reload();
	iwdg_lld_init();
}


//! @}

