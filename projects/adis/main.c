/*! \file main.c
 *
 * Development for ADIS IMU on ChibiOS
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "usbdetail.h"
#include "device_net.h"
#include "fc_net.h"

#include "ADIS16405.h"

static int sendsocket;

void cmd_pid(BaseSequentialStream *chp, int argc __attribute__((unused)), char *argv[]) {
    (void)argv;
    chprintf(chp, "PID: %x\n\r", adis_get(ADIS_PRODUCT_ID));
}

static void adis_drdy_handler(eventid_t id __attribute__((unused)) ){
    BaseSequentialStream *chp = getActiveUsbSerialStream();
    ADIS16405_burst_data data;
    adis_get_data(&data);
    chprintf(chp, "Accel x:%d y:%d z:%d\r\n", data.xaccl_out, data.yaccl_out, data.zaccl_out);
    if(send(sendsocket, &data, sizeof(data), 0) < 0){
         chprintf(chp, "Send socket send failure\r\t");
     }
}

static int get_udp_socket(struct sockaddr *addr){
    //Create the socket
    int s = socket(AF_INET,  SOCK_DGRAM, 0);
    if(s < 0){
        /* socket allocation failure */
        return -1;
    }

    //bind our own address to the socket
    if(bind(s, addr, sizeof(struct sockaddr_in)) < 0){
        /* socket bind failure */
        return -2;
    }
    return s;
}

void main(void) {

	halInit();
	chSysInit();

	adis_init(&adis_olimex_e407);

    static const ShellCommand commands[] = {
            {"pid", cmd_pid},
            {NULL, NULL}
    };
    usbSerialShellStart(commands);

    struct EventListener drdy;
    chEvtRegister(&adis_data_ready, &drdy, 0);

    static const evhandler_t evhndl[] = {
            adis_drdy_handler
    };

    chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 2,
                        lwip_thread, get_adis_addr());

    sendsocket = get_udp_socket(ADIS_IN_ADDR);
    connect(sendsocket, ADIS_OUT_ADDR, sizeof(struct sockaddr_in));

	while(TRUE){
	    chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
