#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_led.h"
void main(void) {

    halInit();
    chSysInit();

    ledStart(NULL);

    lwipThreadStart(GPS_LWIP);
    while(TRUE){
        chThdSleep(TIME_INFINITE);
    }

}

