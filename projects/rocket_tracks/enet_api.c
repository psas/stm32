//******************************************************************************
// enet_api.c
//
//  Created on: Mar 26, 2014
//      Author: Rob Gaskell		PSU ID# 914464617
// 	   Project: RocketTracks
//******************************************************************************

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "usbdetail.h"
#include "utils_sockets.h"

#include "enet_api.h"

static struct sockaddr_in self_addr;
static struct sockaddr_in manual_addr;
static struct sockaddr_in sla_addr;

static int ManualSendSocket;
static int SLASendSocket;
static int SLAReceiveSocket;
static int ManualReceiveSocket;

static ManualData RemoteData;

//Functions to create Ethernet sockets

void SendtoManualSocket(){

    set_sockaddr(&self_addr, IP_MANUAL, MANUAL_TX_PORT);
    ManualSendSocket = get_udp_socket((struct sockaddr*)&self_addr);

    //Create the address to send to
    set_sockaddr(&manual_addr, IP_HOST, DATA_UDP_TX_PORT);
}
void SendtoSLASocket(){

    set_sockaddr(&self_addr, IP_SLA, SLA_TX_PORT);
    SLASendSocket = get_udp_socket((struct sockaddr*)&self_addr);

    //Create the address to send to
    set_sockaddr(&sla_addr, IP_HOST, SLA_TX_PORT);
}
void ReceiveSLASocket() {

    set_sockaddr(&self_addr, IP_SLA, SLA_RX_PORT);
    SLAReceiveSocket = get_udp_socket((struct sockaddr*)&self_addr);
}
void ReceiveManualSocket() {

    set_sockaddr(&self_addr, IP_MANUAL, MANUAL_RX_PORT);
    ManualReceiveSocket = get_udp_socket((struct sockaddr*)&self_addr);
}

//Functions to send Ethernet messages

void SendSLA(int Command) {
//TODO Robert send commands to SLA
}

void SendNeutral(Neutral data) {

char msg[2];

	msg[0] = data.latNeutral;
	msg[1] = data.vertNeutral;

    if(sendto(ManualSendSocket, msg, sizeof(msg), 0, (struct sockaddr*)&manual_addr, sizeof(manual_addr)) < 0){
		#ifndef NDEBUG
        chprintf(chp, "Send socket send failure\r\t");
		#endif
    }
    return;
}

void ReceiveSLA(SLAData data) {

}

/*!
 * Stack area for the rtx_controller_receive_thread.
 */
WORKING_AREA(wa_rtx_controller_receive_thread, RTX_RECEIVE_THREAD_STACK_SIZE);

msg_t rtx_controller_receive_thread(void *p __attribute__ ((unused))) {
    /*
     * This thread creates a UDP socket and then listens for any incoming
     * message, printing it out over serial USB
     */

char msg[MANUAL_REMOTE_MESSAGE_SIZE];
char SLA[SLA_MESSAGE_SIZE];

	chRegSetThreadName("rtx_controller_receive_thread");
	#ifndef NDEBUG
	BaseSequentialStream *chp = getUsbStream();
	#endif

    while(TRUE) {
		if(recvfrom(ManualReceiveSocket, msg, sizeof(msg), 0, (struct sockaddr*)&manual_addr, sizeof(manual_addr)) < 0){
			#ifndef NDEBUG
			chprintf(chp, "Manual socket recv failure \r\n");
			#endif
		}
		else {
			#ifndef NDEBUG
			chprintf(chp, "%s\r\n", msg);
			#endif
			//TODO parse msg into fields of data struct
			RemoteData.Enable = msg[0];
			RemoteData.Mode = msg[1];
			RemoteData.Aux = msg[2];
			temp = (uint16_t)msg[3] << 8;
			RemoteData.latPosition = (uint16_t)msg[4] | temp;
		}
		if(RemoteData.Mode == '1') {
			if(recvfrom(SLAReceiveSocket, SLA, sizeof(SLA), 0, (struct sockaddr*)&sla_addr, sizeof(sla_addr)) < 0){
				#ifndef NDEBUG
				chprintf(chp, "SLA socket recv failure \r\n");
				#endif
			}
			else {
				#ifndef NDEBUG
				chprintf(chp, "%s\r\n", msg);
				#endif
				//TODO Robert parse SLA into fields of data struct
			}
		}
    }
}
