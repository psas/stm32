/*! \file main.c
 *
 * UDP Datapath experiment - Control
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp Application Control
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "board.h"

#include "device_net.h"
#include "fc_net.h"

#include "main.h"

static WORKING_AREA(waThread_blinker, 64);
/*! \brief Green LED blinker thread
 */
static msg_t Thread_blinker(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

static WORKING_AREA(waThread_indwatchdog, 64);
/*! \brief  Watchdog thread
 */
static msg_t Thread_indwatchdog(void *arg) {
	(void)arg;

	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(250);
	}
	return -1;
}

int main(void) {
	static const evhandler_t evhndl_main[]       = {
			extdetail_WKUP_button_handler
	};
	struct EventListener     el0;

	struct lwipthread_opts   ip_opts;

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	extdetail_init();

	palSetPad(GPIOC, GPIOC_LED);

	/*!
	 * GPIO Pins for generating pulses at data input detect and data output send.
	 * Used for measuring latency timing of data
	 *
	 * \sa board.h
	 */
	palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
	palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
	palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );



	const ShellCommand commands[] = {
	        {"mem", cmd_mem},
	        {"threads", cmd_threads},
	        {NULL, NULL}
	};
	usbSerialShellStart(commands);

	/*! Activates the EXT driver 1. */
	extStart(&EXTD1, &extcfg);

	chThdCreateStatic(waThread_blinker          , sizeof(waThread_blinker)          , NORMALPRIO    , Thread_blinker         , NULL);
	chThdCreateStatic(waThread_indwatchdog      , sizeof(waThread_indwatchdog)      , NORMALPRIO    , Thread_indwatchdog     , NULL);

	/*!
	 * Use a locally administered MAC address second LSbit of MSB of MAC should be 1
	 * Use unicast address LSbit of MSB of MAC should be 0
	 */
	static       uint8_t      ROLL_CTL_macAddress[6]         = ROLL_CTL_MAC_ADDRESS;
	struct       ip_addr      ip, gateway, netmask;
	ROLL_CTL_IP_ADDR(&ip);
	ROLL_CTL_GATEWAY(&gateway);
	ROLL_CTL_NETMASK(&netmask);

	ip_opts.macaddress = ROLL_CTL_macAddress;
	ip_opts.address    = ip.addr;
	ip_opts.netmask    = netmask.addr;
	ip_opts.gateway    = gateway.addr;

	chThdCreateStatic(wa_lwip_thread            , sizeof(wa_lwip_thread)            , NORMALPRIO + 2, lwip_thread            , &ip_opts);
    chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);

	chEvtRegister(&extdetail_wkup_event, &el0, 0);

	while (TRUE) {
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}


//! @}
