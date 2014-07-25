#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "rci.h"

void led(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret UNUSED, void * user UNUSED){
	palTogglePad(GPIOC, GPIOC_LED);
}

void main(void) {

	halInit();
	chSysInit();

	lwipThreadStart(RNH_LWIP);

	struct RCICommand cmds[] = {
		{"#LEDS", led, NULL},
		{NULL}
	};

	RCICreate(cmds);

	while(TRUE){
		chThdSleep(TIME_INFINITE);
	}
}

