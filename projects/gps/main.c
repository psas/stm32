#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "utils_shell.h"
#include "usbdetail.h"

WORKING_AREA(wa_tx, 512);
msg_t tx_thread(void *p UNUSED){
    chRegSetThreadName("tx");
    /*
     * This thread initializes a socket and then sends the message
     * "PSAS Rockets! <num>" every half second over UDP to a host
     * socket.
     */

    int s = get_udp_socket(GPS_OUT_ADDR);
    chDbgAssert(s >= 0, "Couldn't get a tx socket", NULL);

    //Create the address to send to. Since there is only one destination,
    //connect() is used here so we wont have to carry the address through.
    struct sockaddr dest;
    set_sockaddr(&dest, "10.0.0.27", 35000);
    if(connect(s, &dest, sizeof(dest))){
        chDbgPanic("Couldn't connect on tx socket");
    }

    //send data to another socket
    char msg[50];
    for(uint8_t count = 0;; ++count){
        chsnprintf(msg, sizeof(msg), "PSAS Rockets! %d", count);
        if(write(s, msg, sizeof(msg)) < 0){
            chDbgPanic("tx socket write failure");
        }
        chThdSleepMilliseconds(500);

    }
    return -1;
}

void main(void) {

    halInit();
    chSysInit();

    ledStart(NULL);
    const ShellCommand commands[] = {
            {"mem", cmd_mem},
            {"threads", cmd_threads},
            {NULL, NULL}
    };
    usbSerialShellStart(commands);

    lwipThreadStart(GPS_LWIP);

    chThdCreateStatic(wa_tx, sizeof(wa_tx), NORMALPRIO, tx_thread, NULL);

    while(TRUE){
        chThdSleep(TIME_INFINITE);
    }

}

