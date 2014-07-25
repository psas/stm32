/*!
 * \file data_udp.c
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "usbdetail.h"
#include "utils_sockets.h"

#include "data_udp.h"

static struct SeqSocket seqSend = DECL_SEQ_SOCKET(DATA_UDP_MSG_SIZE);
static struct SeqSocket seqRecv = DECL_SEQ_SOCKET(DATA_UDP_MSG_SIZE);

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_send_thread(void *p __attribute__ ((unused))){
	BaseSequentialStream* chp;
	struct sockaddr_in self_addr, dest_addr;
	int s;
	uint8_t count;

	chRegSetThreadName("data_udp_send_thread");

	chp = getUsbStream();

	set_sockaddr((struct sockaddr*)&self_addr, IP_DEVICE, DATA_UDP_TX_THREAD_PORT);
	s = get_udp_socket((struct sockaddr*)&self_addr);
	seq_socket_init(&seqSend, s);

	set_sockaddr((struct sockaddr*)&dest_addr, IP_HOST, DATA_UDP_TX_THREAD_PORT);

	for (count = 0 ;; ++count) {
		chsnprintf((char*)seqSend.buffer, DATA_UDP_MSG_SIZE, "PSAS Rockets! %d", count);

		if (seq_sendto(&seqSend, DATA_UDP_MSG_SIZE, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0)
			chprintf(chp, "Send socket send failure\r\t");
		else
			chprintf(chp, "Send seq packet %lu\r\n", seqSend.seqSend);

		chThdSleepMilliseconds(500);
	}
}

WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_receive_thread(void *p __attribute__ ((unused))) {
	BaseSequentialStream* chp;
	struct sockaddr_in self_addr;
	int s;

	chRegSetThreadName("data_udp_receive_thread");

	chp = getUsbStream();

	set_sockaddr((struct sockaddr*)&self_addr, IP_DEVICE, DATA_UDP_RX_THREAD_PORT);
	s = get_udp_socket((struct sockaddr*)&self_addr);
	seq_socket_init(&seqRecv, s);

	while(TRUE) {
		if (seq_recv(&seqRecv, 0) < 0)
			chprintf(chp, "Receive socket recv failure \r\n");
		else
			chprintf(chp, "%s\r\n", seqRecv.buffer);
	}
}

//! @}

