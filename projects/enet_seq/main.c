/* Tests sequenced socket utilites  */
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#include "lwipthread.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_shell.h"
#include "utils_led.h"

#include "usbdetail.h"

#include "assert.h"

#define DATA_UDP_SEND_THREAD_STACK_SIZE      512
#define DATA_UDP_RECEIVE_THREAD_STACK_SIZE   512

#define DATA_UDP_TX_THREAD_PORT              35000
#define DATA_UDP_RX_THREAD_PORT              35003

#define DATA_UDP_RX_THD_PRIORITY             (LOWPRIO)
#define DATA_UDP_THREAD_PRIORITY             (LOWPRIO + 2)

#define IP_HOST                              "192.168.0.91"
#define IP_DEVICE                            "192.168.0.196"

#define DATA_UDP_MSG_SIZE                    50



WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_send_thread(void *p UNUSED){
	BaseSequentialStream* chp;
	struct SeqSocket sender = DECL_SEQ_SOCKET(DATA_UDP_MSG_SIZE);

	struct sockaddr_in self_addr, dest_addr;
	uint8_t count;

	chRegSetThreadName("data_udp_send_thread");

	chp = getUsbStream();

	set_sockaddr((struct sockaddr*)&self_addr, IP_DEVICE, DATA_UDP_TX_THREAD_PORT);
	seqSocket(&sender, (struct sockaddr*)&self_addr);

	set_sockaddr((struct sockaddr*)&dest_addr, IP_HOST, DATA_UDP_TX_THREAD_PORT);

	for (count = 0 ;; ++count) {
		chsnprintf((char*)sender.buffer, DATA_UDP_MSG_SIZE, "PSAS Rockets! %d", count);

		if (seqSendto(&sender, DATA_UDP_MSG_SIZE, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0)
			chprintf(chp, "Send socket send failure\r\t");
		else
			chprintf(chp, "Send seq packet %lu\r\n", sender.seqSend);

		chThdSleepMilliseconds(500);
	}
}

WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_receive_thread(void *p UNUSED) {
	BaseSequentialStream* chp;

	struct SeqSocket recver = DECL_SEQ_SOCKET(DATA_UDP_MSG_SIZE);
	struct sockaddr_in self_addr;

	chRegSetThreadName("data_udp_receive_thread");

	chp = getUsbStream();

	set_sockaddr((struct sockaddr*)&self_addr, IP_DEVICE, DATA_UDP_RX_THREAD_PORT);
	seqSocket(&recver, (struct sockaddr*)&self_addr);

	while(TRUE) {
		if (seqRecv(&recver, 0) < 0)
			chprintf(chp, "Receive socket recv failure \r\n");
		else
			chprintf(chp, "%s\r\n", recver.buffer);
	}
}


int assertFail;

void cmd_assert(BaseSequentialStream *chp UNUSED, int argc UNUSED, char *argv[] UNUSED) {
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

