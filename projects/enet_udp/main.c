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
#include "usbdetail.h"
#include "cmddetail.h"
#include "data_udp.h"



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

	const ShellCommand commands[] = {
	        {"mem", cmd_mem},
	        {"threads", cmd_threads},
	        {NULL, NULL}
	};
	usbSerialShellStart(commands);
	BaseSequentialStream * chp = getActiveUsbSerialStream();

	/* fill out lwipthread_opts with our address*/
    struct lwipthread_opts   ip_opts;
	uint8_t macAddress[6] = {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46};
	set_lwipthread_opts(&ip_opts, IP_DEVICE, "255.255.255.0", "192.168.1.1", macAddress);

	/* Start the lwip thread*/
	chprintf(chp, "LWIP ");
	chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2,
	                    lwip_thread, &ip_opts);

	/* Start the feature threads */
	chprintf(chp, "tx ");
    chThdCreateStatic(wa_data_udp_send_thread, sizeof(wa_data_udp_send_thread), NORMALPRIO,
    		data_udp_send_thread, NULL);
    chprintf(chp, "rx ");
    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO,
      		data_udp_receive_thread, NULL);
    chprintf(chp, "shell ");
    chThdCreateStatic(waThread_blinker, sizeof(waThread_blinker), NORMALPRIO, Thread_blinker, NULL);

    while (TRUE) {
		chThdSleep(TIME_INFINITE);
	}
}


//! @}
