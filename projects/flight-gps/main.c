#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "rnet_cmd_interp.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_led.h"


void main(void) {

	halInit();
	chSysInit();
	ledStart(NULL);
	lwipThreadStart(GPS_LWIP);

	struct RCICommand commands[] = {
		RCI_CMD_VERS
	};
	RCICreate(commands);

	while(TRUE){
		chThdSleep(TIME_INFINITE);
	}
}

