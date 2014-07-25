/* experiments with UDP
 */

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#include "lwipthread.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_shell.h"
#include "utils_led.h"

#include "usbdetail.h"

#define IP_SELF "192.168.0.192"
#define MAC_ADDR (uint8_t[6]){0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46}
#define PORT_TX 35000
#define PORT_RX 35001

#define IP_DEST "192.168.0.191"
#define PORT_DEST 35001

#define MSG_SIZE 50

// FIXME: assert errno

WORKING_AREA(wa_tx, 512);
msg_t tx_thread(void *p UNUSED){
	chRegSetThreadName("tx");
	/*
	 * This thread initializes a socket and then sends the message
	 * "PSAS Rockets! <num>" every half second over UDP to a host
	 * socket.
	 */
	struct sockaddr self;

	set_sockaddr(&self, IP_SELF, PORT_TX);
	int s = get_udp_socket(&self);
	chDbgAssert(s >= 0, "Couldn't get a tx socket", NULL);

	//Create the address to send to. Since there is only one destination,
	//connect() is used here so we wont have to carry the address through.
	struct sockaddr dest;
	set_sockaddr(&dest, IP_DEST, PORT_DEST);
	if(connect(s, &dest, sizeof(dest))){
		chDbgPanic("Couldn't connect on tx socket");
	}

	//send data to another socket
	char msg[MSG_SIZE];
	for(uint8_t count = 0;; ++count){
		chsnprintf(msg, sizeof(msg), "PSAS Rockets! %d", count);
	if(write(s, msg, sizeof(msg)) < 0){
		// Tue 10 June 2014 20:52:24 (PDT) Should we have a panic?
		chDbgPanic("tx socket write failure");
	}
		chThdSleepMilliseconds(500);

	}
	return -1;
}


WORKING_AREA(wa_rx, 512);
msg_t rx_thread(void *p UNUSED) {
	chRegSetThreadName("rx");
	/*
	 * This thread creates a UDP socket and then listens for any incomming
	 * message, printing it out over serial USB
	 */

	BaseSequentialStream *chp = getUsbStream();
	struct sockaddr self;

	set_sockaddr(&self, IP_SELF, PORT_RX);
	int s = get_udp_socket(&self);
	chDbgAssert(s >= 0, "Couldn't get rx socket", NULL);

	//read data from socket
	char msg[MSG_SIZE];
	while(TRUE) {
		//read is used here because we don't care where the packets came from.
		//If we did, recvfrom() is the function to use
		if(read(s, msg, sizeof(msg)) < 0){
			chDbgPanic("Receive socket recv failure");
		} else {
			chprintf(chp, "%s\r\n", msg);
		}
	}

	return -1;
}


void main(void) {
	/* System initializations */
	halInit();
	chSysInit();

	/* Start diagnostics led */
	ledStart(NULL);

	/* Start diagnostics shell */
	const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
	};
	usbSerialShellStart(commands);
	BaseSequentialStream * chp = getUsbStream();

	/* fill out lwipthread_opts with our address*/
	struct lwipthread_opts ip_opts;
	set_lwipthread_opts(&ip_opts, IP_SELF, "255.255.255.0", "0.0.0.0", MAC_ADDR);

	/* Start the lwip thread*/
	chprintf(chp, "LWIP ");
	lwipThreadStart(&ip_opts);

	/* Start the feature threads */
	chprintf(chp, "tx ");
	chThdCreateStatic(wa_tx, sizeof(wa_tx), NORMALPRIO, tx_thread, NULL);
	chprintf(chp, "rx ");
	chThdCreateStatic(wa_rx, sizeof(wa_rx), NORMALPRIO, rx_thread, NULL);

	chprintf(chp, "\r\n");
	while (TRUE) {
		chThdSleep(TIME_INFINITE);
	}
}
