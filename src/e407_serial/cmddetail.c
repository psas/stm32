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
#include "mac.h"
#include "mac_lld.h"
#include "cmddetail.h"
#include "stm32f4xx.h"
#include "chprintf.h"
#include "mii.h"
#include "cmddetail.h"

#define DEBUG_PHY 1

#if DEBUG_PHY

/*! \brief   Read a PHY register.
 *
 */
static uint32_t mii_read(MACDriver *macp, uint32_t reg) {

 ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR | ETH_MACMIIAR_MB;
 while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
   ;
 return ETH->MACMIIDR;
}

/*! \brief   Writes a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 * @param[in] value     new register value
 */
static void mii_write(MACDriver *macp, uint32_t reg, uint32_t value) {

  ETH->MACMIIDR = value;
  ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR |
                  ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;
  while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
	  ;
}


/**
 * @brief   PHY address detection.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 */
void cmd_find_phy(BaseSequentialStream *chp, int argc, char *argv[]) {
	uint32_t i;
	MACDriver *macp = &ETHD1;
	uint32_t physid1 = 0;
	uint32_t physid2 = 0;
	(void)argc;
	(void)argv;
	// 0x00221610

	//BOARD_PHY_ADDRESS
	//chprintf(chp, "BOARD_PHY_ADDRESS:\t0x%x\r\n", (BOARD_PHY_ADDRESS));
	chprintf(chp, "BOARD_PHY_ID>>16:\t0x%x\r\n", (BOARD_PHY_ID>>16));
	chprintf(chp, "BOARD_PHY_ID&0xFFF0\t0x%x\r\n", (BOARD_PHY_ID & 0xFFF0));
	chprintf(chp, "MACMIIDR_CR: 0x%x\tmacp->phyaddr: 0x%x\t\r\n",MACMIIDR_CR, macp->phyaddr);

	chprintf(chp, "phy1_g:\t0x%x\tphy2_g:\t0x%x\r\n", phy1_g, phy2_g);

//	 if ((mii_read(macp, MII_PHYSID1) == (BOARD_PHY_ID >> 16)) &&
//	          ((mii_read(macp, MII_PHYSID2) & 0xFFF0) == (BOARD_PHY_ID & 0xFFF0))) {

#if STM32_MAC_PHY_TIMEOUT > 0

	halrtcnt_t start = halGetCounterValue();
	halrtcnt_t timeout  = start + MS2RTT(STM32_MAC_PHY_TIMEOUT);
	while (halIsCounterWithin(start, timeout)) {
//		chprintf(chp, ".\r\n");
#endif


//		for (i = 0; i < 31; i++) {
//			chprintf(chp, "\r\ni:\t%d\t", i);
//			macp->phyaddr = i << 11;
//			ETH->MACMIIDR = (macp->phyaddr << 6) | MACMIIDR_CR;
//			physid1 = mii_read(macp, MII_PHYSID1);
//			physid2 = mii_read(macp, MII_PHYSID2);
//
//			chprintf(chp, "macp->phyaddr: 0x%x\tphysid1: 0x%x\tphysid2:\t0x%x\r\n",macp->phyaddr, physid1, physid2);
//
//		}
#if STM32_MAC_PHY_TIMEOUT > 0
}
#endif

}

//void cmd_phy_write(BaseSequentialStream *chp, int argc, char *argv[]) {
//
//	//	mii_write(&ETHD1, MII_BMCR, (bmcr_val & ~(1<<12)) );
//	//
//}

void cmd_phy_read(BaseSequentialStream *chp, int argc, char *argv[]) {
	uint32_t phy_val     = 0;
	uint32_t reg_to_ping = 0;
	//	uint32_t bmcr_val = 0;

	if (argc == 0) {
		goto ERROR;
	}
	chprintf(chp, "argv0: %s\r\n", argv[0]);

	if ((argc == 1) && (strcmp(argv[0], "read") == 0)){


		if (argc != 1) {
			chprintf(chp, "Usage: phy reg(decimal)\r\n");
			return;
		}

		//	bmcr_val = mii_read(&ETHD1, MII_BMCR);
		//
		//	bmcr_val = mii_read(&ETHD1, MII_BMCR);
		//
		//	mii_write(&ETHD1, 0x1f,( bmcr_val | 1<<13));
		chprintf(chp, "here\r\n");
		reg_to_ping = atoi(argv[0]);
		phy_val = mii_read(&ETHD1, reg_to_ping);
		chprintf(chp, "phy reg 0x%x value:\t0x%x\n\r", reg_to_ping, phy_val);

		ERROR:
		chprintf(chp, "Usage: phy_read reg\r\n");
		chprintf(chp, "where reg is hex address of register\r\n");
		return;
	}
}
#endif


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
