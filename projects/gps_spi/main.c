#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "rnet_cmd_interp.h"
#include "utils_rci.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "utils_shell.h"
#include "usbdetail.h"

#include "MAX2769.h"

static struct SeqSocket gps_socket = DECL_SEQ_SOCKET(sizeof(MAX2769Data));

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

static void max_write_done_handler(eventid_t id UNUSED){
}

void main(void) {

    halInit();
    chSysInit();

    ledStart(NULL);

    static struct RCIConfig conf;

    conf.commands = (struct RCICommand[]){
            RCI_CMD_VERS,
            {NULL}
    };
    conf.address = SENSOR_RCI_ADDR;

    lwipThreadStart(GPS_LWIP);
    RCICreate(&conf);

    /* Create the GPS out socket, connecting as it only sends to one place */
    int s = get_udp_socket(GPS_OUT_ADDR);
    chDbgAssert(s >= 0, "GPS socket failed", NULL);
    seq_socket_init(&gps_socket, s);

    connect(gps_socket.socket, FC_ADDR, sizeof(struct sockaddr));

    max2769_init(&max2769_gps);

    const ShellCommand commands[] = {
            {"mem", cmd_mem},
            {"threads", cmd_threads},
            {NULL, NULL}
    };
    usbSerialShellStart(commands);

    chThdCreateStatic(wa_tx, sizeof(wa_tx), NORMALPRIO, tx_thread, NULL);

    max2769_config();
    /* Manage MAX2769 events */
    struct EventListener ddone;
    static const evhandler_t evhndl[] = {
           max_write_done_handler
    };
    chEvtRegister(&MAX2769_write_done, &ddone, 0);
    while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
    }

}

