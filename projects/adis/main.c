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
    char msg[3];
    if(send(sendsocket, msg, sizeof(msg), 0) < 0){
         chprintf(chp, "Send socket send failure\r\t");
     }
}

static int get_udp_socket(const char * ip, unsigned int port){
    //Create our own address (remember to have the data in network byte order)
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET,
    addr.sin_port = htons(port);
    inet_aton(ip, &addr.sin_addr);

    //Create the socket
    int s = socket(AF_INET,  SOCK_DGRAM, 0);
    if(s < 0){
        /* socket allocation failure */
        return -1;
    }

    //bind our own address to the socket
    if(bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
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


    struct ip_addr ip, gateway, netmask;
    IP4_ADDR(&ip,      192, 168, 0,   196);
    IP4_ADDR(&gateway, 192, 168, 1,   1  );
    IP4_ADDR(&netmask, 255, 255, 255, 0  );
    uint8_t macAddress[6] = {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46};

    struct lwipthread_opts ip_opts;
    ip_opts.macaddress = macAddress;
    ip_opts.address    = ip.addr;
    ip_opts.netmask    = netmask.addr;

    chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 2,
                        lwip_thread, &ip_opts);

    sendsocket = get_udp_socket(IMU_A_IP_ADDR_STRING, IMU_A_TX_PORT);
    //Create the address to send to (remember to have the data in network byte order)
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(FC_LISTEN_PORT_IMU_A);
    inet_aton(IP_PSAS_FC, &dest_addr.sin_addr);

    connect(sendsocket, &dest_addr, sizeof(struct sockaddr_in));

	while(TRUE){
	    chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
