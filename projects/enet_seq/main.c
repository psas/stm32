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

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#include "lwipthread.h"
#include "lwip/ip_addr.h"

#include "utils_sockets.h"
#include "utils_shell.h"
#include "utils_led.h"

#include "usbdetail.h"
#include "data_udp.h"

#include "assert.h"

int assertFail;

void cmd_assert(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)chp;
	(void)argc;
	(void)argv;

	assertFail = 1;
}

void main(void) {
	BaseSequentialStream* chp;
	const ShellCommand commands[] = {
		{ "assert", cmd_assert },
		{ "mem", cmd_mem },
		{ "threads", cmd_threads },

		{ NULL, NULL }
	};
	struct lwipthread_opts ip_opts;
	uint8_t macAddress[6] = { 0xc2, 0xaf, 0x51, 0x03, 0xcf, 0x46 };

	halInit();
	chSysInit();

	/* Start diagnostics led */
	ledStart(NULL);

	/* Start diagnostics shell */
	usbSerialShellStart(commands);
	chp = getUsbStream();

	/* fill out lwipthread_opts with our address*/
	set_lwipthread_opts(&ip_opts, IP_DEVICE, "255.255.255.0", "192.168.1.1", macAddress);

	/* Start the lwip thread*/
	chprintf(chp, "LWIP ");
	lwipThreadStart(&ip_opts);

	/* Start the worker threads */
	chprintf(chp, "tx ");
	chThdCreateStatic(wa_data_udp_send_thread, sizeof(wa_data_udp_send_thread), NORMALPRIO, data_udp_send_thread, NULL);

	chprintf(chp, "rx ");
	chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO, data_udp_receive_thread, NULL);

	while (TRUE) {
		chThdSleep(100);

		assert(!assertFail);
	}
}


//! @}
