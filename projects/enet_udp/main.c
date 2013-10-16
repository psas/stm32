/*! \file main.c
 *
 * experiments with UDP
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp Application
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "cmddetail.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipthread.h"

static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU_PSAS,
		commands
};

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

void main(void) {
	static Thread            *shelltp       = NULL;
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

	palSetPad(GPIOC, GPIOC_LED);

	/*!
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU_PSAS);
	sduStart(&SDU_PSAS, &serusbcfg);

	/*!
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	shellInit();

	struct ip_addr ip, gateway, netmask;
	IP4_ADDR(&ip,      192, 168, 0,   196);
	IP4_ADDR(&gateway, 192, 168, 1,   1  );
	IP4_ADDR(&netmask, 255, 255, 255, 0  );
	uint8_t macAddress[6] = {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46};

	ip_opts.macaddress = macAddress;
	ip_opts.address    = ip.addr;
	ip_opts.netmask    = netmask.addr;
	ip_opts.gateway    = gateway.addr;
	chprintf((struct BaseSequentialStream *)&SDU_PSAS, "LWIP ");
	chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 2,
	                    lwip_thread, &ip_opts);
	chprintf((struct BaseSequentialStream *)&SDU_PSAS, "tx ");
    chThdCreateStatic(wa_data_udp_send_thread, sizeof(wa_data_udp_send_thread), NORMALPRIO,
    		data_udp_send_thread, NULL);
    chprintf((struct BaseSequentialStream *)&SDU_PSAS, "rx ");
    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO,
      		data_udp_receive_thread, NULL);
    chprintf((struct BaseSequentialStream *)&SDU_PSAS, "shell ");
    chThdCreateStatic(waThread_blinker, sizeof(waThread_blinker), NORMALPRIO, Thread_blinker, NULL);
	while (TRUE) {
		if (!shelltp && (SDU_PSAS.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
	}
}


//! @}
