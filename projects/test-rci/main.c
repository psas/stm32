#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "rnet_cmd_interp.h"

#define UNUSED __attribute__((unused))

void led(struct RCICmdData *rci_data UNUSED, void * user_data UNUSED){
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

    struct RCIConfig cfg = {
	    .address = RNH_RCI_ADDR,
	    .commands = cmds
    };
    
    RCICreate(&cfg);

    while(TRUE){
        chThdSleep(TIME_INFINITE);
    }
}

