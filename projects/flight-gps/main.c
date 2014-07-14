#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "rci.h"
#include "utils_rci.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_led.h"

static struct SeqSocket cots_socket = DECL_SEQ_SOCKET(400);

static void cots_handler(eventid_t id UNUSED){
	static int len = 0;

	len += sdAsynchronousRead(&SD6, cots_socket.buffer + len, MAX(400 - len, 0));
//	if((cots_socket.buffer[MAX(len - 2, 0)] == '\r' && cots_socket.buffer[MAX(len - 1, 0)] == '\n') || len >= 400){
		seq_write(&cots_socket, len);
		len = 0;
//	}
}

void main(void) {

	halInit();
	chSysInit();
	ledStart(NULL);
	lwipThreadStart(GPS_LWIP);

	struct RCICommand commands[] = {
		RCI_CMD_VERS
	};
	RCICreate(commands);

	sdStart(&SD6, NULL);

	int s = get_udp_socket(GPS_COTS_ADDR);
	chDbgAssert(s >= 0, "COTS socket failed", NULL);
	seq_socket_init(&cots_socket, s);
	connect(cots_socket.socket, FC_ADDR, sizeof(struct sockaddr));



	struct EventListener elCots;
	static const evhandler_t evhndl[] = {
		cots_handler
	};
	chEvtRegister(chnGetEventSource(&SD6), &elCots, 0);


	while(TRUE){
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}

