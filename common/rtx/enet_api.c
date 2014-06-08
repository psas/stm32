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
#include "rocket_tracks.h"
#include "net_addrs.h"

#define ENABLE_MSG		0
#define MODE_MSG		1
#define AUX_MSG			2
#define LAT_MSG			3
#define VERT_MSG		5
#define AXIS3_MSG		7
#define AXIS4_MSG		9
#define BYTE_MASK		255


int RTxtoManualSendSocket;
int ManualtoRTxSendSocket;

int RTxfromSLAReceiveSocket;
int ManualReceiveSocket;
int NeutralReceiveSocket;

//Functions to create Ethernet sockets

void SendRTxtoManualSocket(){

    RTxtoManualSendSocket = get_udp_socket(RTX_NEUTRAL_ADDR);
    chDbgAssert(RTxtoManualSendSocket >=0, "Neutral socket failed", NULL);

    //Create the address to send to
    if(connect(RTxtoManualSendSocket, RTXMAN_NEUTRAL_ADDR, sizeof(struct sockaddr))){
        chDbgPanic("Couldn't connect on tx socket");
    }
}

void SendManualtoRTxSocket(){

	ManualtoRTxSendSocket = get_udp_socket(RTXMAN_OUT_ADDR);
    chDbgAssert(ManualtoRTxSendSocket >=0, "Manual socket failed", NULL);

    //Create the address to send to
    if(connect(ManualtoRTxSendSocket, RTX_MANUAL_ADDR, sizeof(struct sockaddr))){
        chDbgPanic("Couldn't connect on tx socket");
    }
}

void ReceiveRTxfromSLASocket() {

	RTxfromSLAReceiveSocket = get_udp_socket(RTX_FROMSLA_ADDR);
    chDbgAssert(RTxfromSLAReceiveSocket >=0, "SLA socket failed", NULL);

}

// Create thread for RTx to receive Manual Status from Manual Control Box
void ReceiveRTxfromManualSocket() {

	ManualReceiveSocket = get_udp_socket(RTX_MANUAL_ADDR);
    chDbgAssert(ManualReceiveSocket >=0, "Manual socket failed", NULL);

    //Create the address to send to
    if(connect(ManualReceiveSocket, RTXMAN_OUT_ADDR, sizeof(struct sockaddr))){
        chDbgPanic("Couldn't connect on tx socket");
    }
}

// Create thread for Manual Control Box to receive Neutral Status from RTx
void ReceiveManualfromRTxSocket() {

	NeutralReceiveSocket = get_udp_socket(RTXMAN_NEUTRAL_ADDR);
    chDbgAssert(NeutralReceiveSocket >=0, "Neutral socket failed", NULL);

    //Create the address to receive from
    if(connect(NeutralReceiveSocket, RTX_NEUTRAL_ADDR, sizeof(struct sockaddr))){
        chDbgPanic("Couldn't connect on tx socket");
    }
}

//Functions to send Ethernet messages

void SendNeutral(Neutral * data) {

char msg[sizeof(*data)];
#ifndef NDEBUG
BaseSequentialStream *chp = getUsbStream();
#endif

	msg[LAT_AXIS] = (char)data->latNeutral;
	msg[VERT_AXIS] = (char)data->vertNeutral;

    if(write(RTxtoManualSendSocket, msg, sizeof(msg)) < 0){
		#ifndef NDEBUG
        chprintf(chp, "Neutral Send socket send failure\r\t");
		#endif
    }
    return;
}

void SendManual(ManualData * data) {

char msg[sizeof(*data)];
uint8_t temp = 0;
#ifndef NDEBUG
BaseSequentialStream *chp = getUsbStream();
#endif

	// Insert Manual Control Box switch status
	msg[ENABLE_MSG] = (char)data->Enable;
	msg[MODE_MSG] = (char)data->Mode;
	msg[AUX_MSG] =(char) data->Aux;

	// Separate bytes of Lat Axis Desired Position and insert in msg
	temp = (uint8_t)(BYTE_MASK & data->latPosition);
	msg[LAT_MSG] = (char)temp;
	temp = (uint8_t)(BYTE_MASK & (data->latPosition >> 8));
	msg[LAT_MSG + 1] = (char)temp;

	// Separate bytes of Vert Axis Desired Position and insert in msg
	temp = (uint8_t)(BYTE_MASK & data->vertPosition);
	msg[VERT_MSG] = (char)temp;
	temp = (uint8_t)(BYTE_MASK & (data->vertPosition >> 8));
	msg[VERT_MSG + 1] = (char)temp;

	// Separate bytes of Axis3 Desired Position and insert in msg
	temp = (uint8_t)(BYTE_MASK & data->Axis3Position);
	msg[AXIS3_MSG] = (char)temp;
	temp = (uint8_t)(BYTE_MASK & (data->Axis3Position >> 8));
	msg[AXIS3_MSG + 1] = (char)temp;

	// Separate bytes of Axis4 Desired Position and insert in msg
	temp = (uint8_t)(BYTE_MASK & data->Axis4Position);
	msg[AXIS4_MSG] = (char)temp;
	temp = (uint8_t)(BYTE_MASK & (data->Axis4Position >> 8));
	msg[AXIS4_MSG + 1] = (char)temp;

    if(write(ManualtoRTxSendSocket, msg, sizeof(msg)) < 0){
		#ifndef NDEBUG
        chprintf(chp, "Send socket send failure\r\t");
		#endif
    }
    return;
}

int ReceiveManual(ManualData * data) {

char msg[sizeof(*data)];
uint16_t temp = 0;
//#ifndef NDEBUG
//BaseSequentialStream *chp = getUsbStream();
//#endif

	if(read(ManualReceiveSocket, msg, sizeof(msg)) < 0){
//		#ifndef NDEBUG
//		chprintf(chp, "Manual socket recv failure \r\n");
//		#endif
		return -1;
	}
	else {
//		#ifndef NDEBUG
//		chprintf(chp, "%s\r\n", msg);
//		#endif
		//Parse msg into fields of data struct
		data->Enable = (uint8_t)msg[ENABLE_MSG];
		data->Mode = (uint8_t)msg[MODE_MSG];
		data->Aux = (uint8_t)msg[AUX_MSG];

		temp = (uint16_t)msg[LAT_MSG] << 8;
		data->latPosition = (uint16_t)msg[LAT_MSG + 1] | temp;
		temp = (uint16_t)msg[VERT_MSG] << 8;
		data->vertPosition = (uint16_t)msg[VERT_MSG + 1] | temp;
		temp = (uint16_t)msg[AXIS3_MSG] << 8;
		data->Axis3Position = (uint16_t)msg[AXIS3_MSG + 1] | temp;
		temp = (uint16_t)msg[AXIS4_MSG] << 8;
		data->Axis4Position = (uint16_t)msg[AXIS4_MSG + 1] | temp;

		return 1;
    }
}

void ReceiveNeutral(Neutral * data) {

char msg[sizeof(*data)];
//#ifndef NDEBUG
//BaseSequentialStream *chp = getUsbStream();
//#endif

    if(read(NeutralReceiveSocket, msg, sizeof(msg)) < 0){
//		#ifndef NDEBUG
//        chprintf(chp, "Neutral socket recv failure \r\n");
//		#endif
    }
	data->latNeutral = (uint8_t)msg[LAT_AXIS];
	data->vertNeutral = (uint8_t)msg[VERT_AXIS];

    return;
}

void ReceiveSLA(SLAData * data) {

	char msg[20];
	uint16_t temp = 0;
	#ifndef NDEBUG
	BaseSequentialStream *chp = getUsbStream();
	#endif

		msg[LAT_AXIS] = '\0';
		msg[VERT_AXIS] = '\0';

	    if(read(NeutralReceiveSocket, msg, sizeof(msg)) < 0){
			#ifndef NDEBUG
	        chprintf(chp, "Neutral socket recv failure \r\n");
			#endif
	    }
		temp = (uint16_t)msg[SLA_COLUMN_OFFSET] << 8;
		data->Column = (uint16_t)msg[SLA_COLUMN_OFFSET + 1] | temp;
		temp = (uint16_t)msg[SLA_ROW_OFFSET] << 8;
		data->Row = (uint16_t)msg[SLA_ROW_OFFSET + 1] | temp;

	    return;
}
