
#include <string.h>
#include <stdint.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lwip/inet.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "data_udp.h"

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);


static uint8_t ip_addr_1(uint32_t addr) {
	return (addr & 0xff);
}

static uint8_t ip_addr_2(uint32_t addr) {
	return (addr & 0xff00) >> 8;
}

static uint8_t ip_addr_3(uint32_t addr) {
	return (addr & 0xff0000) >> 16;
}

static uint8_t ip_addr_4(uint32_t addr) {
	return (addr & 0xff000000) >> 24;
}

static void print_ip(uint32_t addr, uint16_t port) {
	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;

	chprintf(chp, "my_ip: %u.%u.%u.%u: %u\r\n", ip_addr_1(addr), ip_addr_2(addr), ip_addr_3(addr), ip_addr_4(addr), port);

}

msg_t data_udp_send_thread(void *p __attribute__ ((unused))) {
	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;
	uint8_t               count = 0;
	char                   msg[DATA_UDP_MSG_SIZE] ;

	struct sockaddr_in my_addr;
	socklen_t my_addr_len = sizeof(my_addr);

	chRegSetThreadName("data_udp_send_thread");

	chprintf(chp, "Start udp send thread\n\r");

    int s = get_udp_socket(RNH_BATTERY_ADDR);
    if(s < 0){
        return -1;
    }
    connect(s, FC_ADDR, sizeof(struct sockaddr));

    getsockname(s, (struct sockaddr *)&my_addr, &my_addr_len);
	print_ip(my_addr.sin_addr.s_addr, my_addr.sin_port);

    for( ;; ){
        chsnprintf(msg, sizeof(msg), "rnet tx: %d", count++);
        write(s, msg, sizeof(msg));
        chThdSleepMilliseconds(500);
    }
    return -1;
}

static void data_udp_rx_serve(int socket) {
	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;

	static uint8_t       count  = 0;

    const uint16_t buflen = 50;
	uint8_t inbuf [buflen];

	uint16_t             i      = 0;

    struct sockaddr_in my_addr;
    socklen_t my_addr_len = sizeof(my_addr);
	/*
	 * Read the data from the port, blocking if nothing yet there.
	 * We assume the request (the part we care about) is in one packet
	 */
	read(socket, inbuf, buflen);

    getsockname(socket, (struct sockaddr *)&my_addr, &my_addr_len);
    print_ip(my_addr.sin_addr.s_addr, my_addr.sin_port);

    chprintf(chp, "\r\nsensor rx:%d:->", count++);
    for(i=0; i<buflen; ++i) {
        chprintf(chp, "%c", inbuf[i]);
    }
    chprintf(chp, "\r\n");
}

/*!
 * Stack area for the data_udp_receive_thread.
 */
WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

/*!
 * data_udp_rx  thread.
 */
msg_t data_udp_receive_thread(void *p __attribute__ ((unused))) {
	chRegSetThreadName("data_udp_receive_thread");

	int s = get_udp_socket(RNH_RCI_ADDR);
	if (s < 0) {
	    return -1;
	}
    while(TRUE) {
        data_udp_rx_serve(s);
    }
	return -1;
}
