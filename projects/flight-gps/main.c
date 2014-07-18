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
	int i = 0;
	do {
		len += sdAsynchronousRead(&SD6, cots_socket.buffer + len, 1);
		if(len <= 0){
			return;
		}
		for(; i+1 < len; ++i){
			if((cots_socket.buffer[i] == '\r' && cots_socket.buffer[i + 1] == '\n') || len >= 400){
				seq_write(&cots_socket, len);
				len = 0;
				return;
			}
		}
	} while(TRUE);
}

void main(void) {

	halInit();
	chSysInit();
	//ledStart(NULL);
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
		eventmask_t events = chEvtWaitAny(ALL_EVENTS);
		if(events & 1 << 0){
			ledToggle(&RED);
		} else if(events & 1 << 1) {
			ledToggle(&BLUE);
		} else if(events & 1 << 2) {
			ledToggle(&GREEN);
		} else if(events & 1 << 3) {
			ledToggle(&LED2);
		} else if(events & 1 << 4) {
			ledToggle(&LED4);
		} else if(events & 1 << 5) {
			ledToggle(&LED5);
		}
		chEvtDispatch(evhndl, events);
	}
}

