#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "rci.h"
#include "utils_rci.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_led.h"
#ifdef FLIGHT
#include "iwdg.h"
#endif


#define COTS_BUFFER_SIZE 1400
static struct SeqSocket cots_socket = DECL_SEQ_SOCKET(COTS_BUFFER_SIZE);

void main(void) {

	halInit();
	chSysInit();
#ifdef FLIGHT
	iwdgStart();
#endif
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

	while(TRUE){
		int len = sdReadTimeout(&SD6, cots_socket.buffer, COTS_BUFFER_SIZE, S2ST(1));
		seq_write(&cots_socket, len);
		ledToggle(&RED);
	}
}

