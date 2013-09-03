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


#if DEBUG_KSZ

/*! \brief Assert or deassert all node enables
 *
 * nodes: 1, 2, 3, 4, 6, 7
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_ksz_nodes_en_n(BaseSequentialStream *chp, int argc, char *argv[]) {

	bool      pad_state;

	if(argc == 0) {
		// get current state of pin
		pad_state = palReadPad(GPIOE, GPIO_E0_NODE1_N_EN );
		chprintf(chp, "GPIO_E0_NODE1_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		pad_state = palReadPad(GPIOE, GPIO_E1_NODE2_N_EN );
		chprintf(chp, "GPIO_E1_NODE2_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		pad_state = palReadPad(GPIOE, GPIO_E2_NODE3_N_EN );
		chprintf(chp, "GPIO_E2_NODE3_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		pad_state = palReadPad(GPIOE, GPIO_E3_NODE4_N_EN );
		chprintf(chp, "GPIO_E3_NODE4_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		pad_state = palReadPad(GPIOE, GPIO_E5_NODE6_N_EN );
		chprintf(chp, "GPIO_E5_NODE6_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		pad_state = palReadPad(GPIOE, GPIO_E6_NODE7_N_EN );
		chprintf(chp, "GPIO_E6_NODE7_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "h", 1)  == 0)) {
		goto ERROR;
	}

	if ((argc == 1) && (strncmp(argv[0], "off", 3)  == 0)) {
		// deassert
		chprintf(chp, "deassert\r\n");
		palSetPad(GPIOE, GPIO_E0_NODE1_N_EN);
		palSetPad(GPIOE, GPIO_E1_NODE2_N_EN);
		palSetPad(GPIOE, GPIO_E2_NODE3_N_EN);
		palSetPad(GPIOE, GPIO_E3_NODE4_N_EN);
		palSetPad(GPIOE, GPIO_E5_NODE6_N_EN);
		palSetPad(GPIOE, GPIO_E6_NODE7_N_EN);
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "on", 2) == 0)) {
		// assert
		chprintf(chp, "assert\r\n");
		palClearPad(GPIOE, GPIO_E0_NODE1_N_EN);
		palClearPad(GPIOE, GPIO_E1_NODE2_N_EN);
		palClearPad(GPIOE, GPIO_E2_NODE3_N_EN);
		palClearPad(GPIOE, GPIO_E3_NODE4_N_EN);
		palClearPad(GPIOE, GPIO_E5_NODE6_N_EN);
		palClearPad(GPIOE, GPIO_E6_NODE7_N_EN);
		return;
	}

	ERROR:
	chprintf(chp, "Usage:_ksz_nodes_en_n\r\n");
	chprintf(chp, "       ksz_nodes_en_n <option>\r\n");
	chprintf(chp, "       where option may be 'on' or 'off'\r\n");
	chprintf(chp, "Command without option will return current setting of pins.\r\n");
	return;

}

/*! \brief Assert or deassert reset GPIO_E0_NODE1_N_EN
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_ksz_n1en_n(BaseSequentialStream *chp, int argc, char *argv[]) {

	bool      pad_state;

	if(argc == 0) {
		// get current state of pin
		pad_state = palReadPad(GPIOE, GPIO_E0_NODE1_N_EN );
		chprintf(chp, "GPIO_E0_NODE1_N_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "h", 1)  == 0)) {
		goto ERROR;
	}

	if ((argc == 1) && (strncmp(argv[0], "off", 3)  == 0)) {
		// deassert
		chprintf(chp, "deassert\r\n");
		palSetPad(GPIOE, GPIO_E0_NODE1_N_EN);
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "on", 2) == 0)) {
		// assert
		chprintf(chp, "assert\r\n");
		palClearPad(GPIOE, GPIO_E0_NODE1_N_EN);
		return;
	}

	ERROR:
	chprintf(chp, "Usage: ksz_n1en_n\r\n");
	chprintf(chp, "       ksz_n1en_n <option>\r\n");
	chprintf(chp, "       where option may be 'on' or 'off'\r\n");
	chprintf(chp, "Command without option will return current setting of GPIO_E0_NODE1_EN.\r\n");
	return;

}


/*! \brief Assert or deassert reset GPIO_D4_ETH_N_RST
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_ksz_rst_n(BaseSequentialStream *chp, int argc, char *argv[]) {
	bool      pad_state;

	if(argc == 0) {
		// get current state of pin
		pad_state = palReadPad(GPIOD, GPIO_D4_ETH_N_RST);
		chprintf(chp, "GPIO_D4_ETH_N_RST:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "h", 1)  == 0)) {
		goto ERROR;
	}

	if ((argc == 1) && (strncmp(argv[0], "off", 3)  == 0)) {
		// deassert
		chprintf(chp, "deassert\r\n");
		palSetPad(GPIOD, GPIO_D4_ETH_N_RST);
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "on", 2) == 0)) {
		// assert
		chprintf(chp, "assert\r\n");
		palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
		return;
	}

	ERROR:
	chprintf(chp, "Usage: ksz_rst_n\r\n");
	chprintf(chp, "       ksz_rst_n <option>\r\n");
	chprintf(chp, "       where option may be 'on' or 'off'\r\n");
	chprintf(chp, "Command without option will return current setting of GPIO_D4_ETH_N_RST.\r\n");
	return;
}




/*! \brief Power on or off KSZ through GPIO_D14_KSZ_EN
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_ksz_pwr(BaseSequentialStream *chp, int argc, char *argv[]) {
	bool      pad_state;

	if(argc == 0) {
		// get current state of pin
		pad_state = palReadPad(GPIOD, GPIO_D14_KSZ_EN);
		chprintf(chp, "GPIO_D14_KSZ_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "h", 1)  == 0)) {
		goto ERROR;
	}

	if ((argc == 1) && (strncmp(argv[0], "on", 2)  == 0)) {
		// set to on
		palSetPad(GPIOD, GPIO_D14_KSZ_EN);
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "off", 3) == 0)) {
		// set to off
		palClearPad(GPIOD, GPIO_D14_KSZ_EN);
		return;
	}

	ERROR:
	chprintf(chp, "Usage: ksz_pwr\r\n");
	chprintf(chp, "       ksz_pwr <option>\r\n");
	chprintf(chp, "       where option may be 'on' or 'off'\r\n");
	chprintf(chp, "Command without option will return current setting of GPIO_D14_KSZ_EN.");
	return;
}
#endif


/*! \brief Show reg
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_show(BaseSequentialStream *chp, int argc, char *argv[]) {
	//uint32_t i = 0;
	//MACDriver *macp = &ETHD1;

	(void)argv;
    (void)argc;

    chprintf(chp, "I am the Rocketnet Hub board.\r\n");

	chprintf(chp, "RCC->CFGR : 0x%x\r\n", RCC->CFGR);
//	chprintf(chp, "BOARD_PHY_ID>>16:\t0x%x\r\n", (BOARD_PHY_ID>>16));
//	chprintf(chp, "BOARD_PHY_ID&0xFFF0\t0x%x\r\n", (BOARD_PHY_ID & 0xFFF0));
//	chprintf(chp, "MACMIIDR_CR: 0x%x\tmacp->phyaddr: 0x%x\t\r\n",MACMIIDR_CR, macp->phyaddr);

//	for(i=0; i<32; ++i){
//		chprintf(chp, "i: %d\tphy1_g:\t0x%x\tphy2_g:\t0x%x\r\n", i, phy1_g[i], phy2_g[i]);
//	}


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
