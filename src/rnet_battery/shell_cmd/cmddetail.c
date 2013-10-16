/*! \file cmddetail.c
 *
 */

/*!
 * \defgroup cmddetail Command Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "cmddetail.h"
#include "stm32f4xx.h"
#include "chprintf.h"
#include "BQ24725.h"


void cmd_bq_charge(BaseSequentialStream *chp, int argc, char *argv[]){
    BQ24725_charge_options BQ24725_rocket_init = {
                .ACOK_deglitch_time = t150ms,
                .WATCHDOG_timer = disabled,
                .BAT_depletion_threshold = FT70_97pct,
                .EMI_sw_freq_adj = dec18pct,
                .EMI_sw_freq_adj_en = sw_freq_adj_disable,
                .IFAULT_HI_threshold = l700mV,
                .LEARN_en = LEARN_disable,
                .IOUT = adapter_current,
                .ACOC_threshold = l1_66X,
                .charge_inhibit = charge_enable
            };
            BQ24725_SetChargeCurrent(0x400);
            chprintf(chp, "Charge current set\n");
            BQ24725_SetChargeVoltage(0x41A0);
            chprintf(chp, "Charge voltage set\n");
            BQ24725_SetInputCurrent(0x0A00);
            chprintf(chp, "Input Current set\n");
            BQ24725_SetChargeOption(&BQ24725_rocket_init);
            chprintf(chp, "Charge options set\n");
}

void cmd_bq_did(BaseSequentialStream *chp, int argc, char *argv[]){
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: bq_did. Should return 0x8\n");
		return;
	}

	uint16_t data;
	int err = BQ24725_GetDeviceID(&data);
	if(err){
//#define I2CD_BUS_ERROR              0x01   /**< @brief Bus Error.           */
//#define I2CD_ARBITRATION_LOST       0x02   /**< @brief Arbitration Lost.    */
//#define I2CD_ACK_FAILURE            0x04   /**< @brief Acknowledge Failure. */
//#define I2CD_OVERRUN                0x08   /**< @brief Overrun/Underrun.    */
//#define I2CD_PEC_ERROR              0x10   /**< @brief PEC Error in
//                                                reception.                  */
//#define I2CD_TIMEOUT                0x20   /**< @brief Hardware timeout.    */
//#define I2CD_SMB_ALERT              0x40   /**< @brief SMBus Alert.         */
		chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
	}
	else{
		chprintf(chp, "GOT SOMETHING COOL: 0x%x\n", data);
	}
}

void cmd_bq_mid(BaseSequentialStream *chp, int argc, char *argv[]){
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: bq_mid. Should return 0x40\n");
		return;
	}
	uint16_t data;
	int err = BQ24725_GetManufactureID(&data);
	if(err){
		chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
	}
	else{
		chprintf(chp, "GOT SOMETHING COOL: 0x%x\n", data);
	}
}

void cmd_acok(BaseSequentialStream *chp, int argc, char *argv[]){
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: acok\n");
		return;
	}
	int state = palReadPad(GPIOD, GPIO_D0_BQ24_ACOK);
	switch(state){
	case PAL_HIGH:
		chprintf(chp, "high\n");
		break;
	case PAL_LOW:
		chprintf(chp, "low\n");
		break;
	default:
		chprintf(chp, "unknown\n");
	}
}


/*! \brief Show memory usage
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

/*! \brief Show running threads
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
	static const char *states[] = {THD_STATE_NAMES};
	Thread *tp;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: threads\r\n");
		return;
	}
	chprintf(chp, "addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");
	tp = chRegFirstThread();
	do {
		chprintf(chp, "%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n",
				(uint32_t)tp, (uint32_t)tp->p_ctx.r13,
				(uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
				states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
		tp = chRegNextThread(tp);
	} while (tp != NULL);
}

//! @}
