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

#include "iwdg.h"

typedef enum {
	IWDG_PS_DIV4 = 0,
	IWDG_PS_DIV8,
	IWDG_PS_DIV16,
	IWDG_PS_DIV32,
	IWDG_PS_DIV64,
	IWDG_PS_DIV128,
	IWDG_PS_DIV256
} iwdg_ps_val;

/*! \brief Start the independent watchdog timer
 */
static void iwdg_lld_init(void) {
	chSysLock();
	IWDG->KR = (uint16_t)0xCCCC;
	chSysUnlock();
}

/*! \brief Reload the watchdog by writing the key register
 *
 * 'Feed' the watchdog
 */
static void iwdg_lld_reload(void) {
	chSysLock();
	IWDG->KR = (uint16_t)0xAAAA;
	chSysUnlock();
}

/*! \brief Set the IWDG Clock prescale value
 */
static void iwdg_lld_set_prescale(iwdg_ps_val s) {

	chSysLock();
	IWDG->KR = (uint16_t)0x5555;
	IWDG->PR = s;
	IWDG->KR = (uint16_t)0xAAAA;
	chSysUnlock();
}

static WORKING_AREA(wa, 64);
/*! \brief  Watchdog thread
*/
static msg_t wdthread(void *arg) {
	uint16_t timeout = *((uint16_t *)arg);
	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(timeout);
	}
	return -1;
}

/*! \brief check reset status and then start iwatchdog
 *
 * Check the CSR register for reset source then start
 * the independent watchdog counter.
 *
 */

static unsigned int timeout_ms = 250;
//todo: have config struct, calc timeout based on PS_DIV
void iwdgStart(void) {
	// was this a reset caused by the iwdg?
	if( (RCC->CSR & RCC_CSR_WDGRSTF) != 0) {
		//! \todo Log WDG reset event somewhere.
		RCC->CSR = RCC->CSR | RCC_CSR_RMVF;    // clear the IWDGRSTF
	}
	iwdg_lld_set_prescale(IWDG_PS_DIV32);      // This should be about 2 seconds at 32kHz
	//iwdg_lld_set_prescale(IWDG_PS_DIV128);
	iwdg_lld_reload();
	iwdg_lld_init();
	chThdCreateStatic(wa, sizeof(wa), NORMALPRIO, wdthread, &timeout_ms);
}


//! @}

