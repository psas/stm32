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
#define VERT_MSG		4
#define AXIS3_MSG		5
#define AXIS4_MSG		6
#define BYTE_MASK		255

#define MSGU16_LEN		(sizeof(*data)/sizeof(uint16_t))


int RTxtoManualSendSocket;
int ManualtoRTxSendSocket;

int RTxfromSLAReceiveSocket;
int ManualReceiveSocket;
int NeutralReceiveSocket;

int DiagnosticsSendSocket;
int DiagnosticsReceiveSocket;

//Functions to create Ethernet sockets

void SendRTxtoManualSocket(){

    RTxtoManualSendSocket = get_udp_socket(RTX_NEUTRAL_ADDR);
    chDbgAssert(RTxtoManualSendSocket >=0, "Neutral socket failed", NULL);

    //Create the address to send to
    if(connect(RTxtoManualSendSocket, RTXMAN_NEUTRAL_ADDR, sizeof(struct sockaddr))){
        chDbgPanic("Couldn't connect on tx socket");
    }
}

void SendDiagnosticsSocket(){

	DiagnosticsSendSocket = get_udp_socket(RTX_DIAG_ADDR);
    chDbgAssert(DiagnosticsSendSocket >=0, "Neutral socket failed", NULL);

    //Create the address to send to
    if(connect(DiagnosticsSendSocket, RTXMAN_DIAG_ADDR, sizeof(struct sockaddr))){
        chDbgPanic("Couldn't connect on tx socket");
    }
}

void ReceiveDiagnosticsSocket(){

	DiagnosticsReceiveSocket = get_udp_socket(RTXMAN_DIAG_ADDR);
    chDbgAssert(DiagnosticsReceiveSocket >=0, "Neutral socket failed", NULL);

    //Create the address to send to
    if(connect(DiagnosticsReceiveSocket, RTX_DIAG_ADDR, sizeof(struct sockaddr))){
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

    //Create the address to receive from to
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

uint8_t msg[sizeof(*data)];

	msg[LAT_AXIS] = data->latNeutral;
	msg[VERT_AXIS] = data->vertNeutral;

    write(RTxtoManualSendSocket, msg, sizeof(msg));

    return;
}

void SendManual(ManualData * data) {

uint16_t msg[MSGU16_LEN];

	// Insert Manual Control Box switch status
	msg[ENABLE_MSG] = data->Enable;
	msg[MODE_MSG] = data->Mode;
	msg[AUX_MSG] = data->Aux;

	msg[LAT_MSG] = data->latPosition;
	msg[VERT_MSG] = data->vertPosition;
	msg[AXIS3_MSG] = data->Axis3Position;
	msg[AXIS4_MSG] = data->Axis4Position;

    write(ManualtoRTxSendSocket, msg, sizeof(msg));

    return;
}

void SendDiagnostics(Diagnostics * lat, Diagnostics * vert) {

int i;
uint32_t msg[22];
Diagnostics * data;

	for(i = 0; i < 2; ++i) {
		if(i == 0)
			data = lat;
		else
			data = vert;

		msg[0 + (i*11)] = (uint32_t)data->U16FeedbackADC;
		msg[1 + (i*11)] = (uint32_t)data->U16FeedbackADCPrevious;
		msg[2 + (i*11)] = (uint32_t)data->S16OutputCommand;
		msg[3 + (i*11)] = (uint32_t)data->U16PositionDesired;
		msg[4 + (i*11)] = (uint32_t)data->U16PositionActual;
		msg[5 + (i*11)] = (uint32_t)data->S16PositionError;
		msg[6 + (i*11)] = (uint32_t)data->S16PositionErrorPrevious;
		msg[7 + (i*11)] = (uint32_t)data->S32PositionPTerm;
		msg[8 + (i*11)] = (uint32_t)data->S32PositionITerm;
		msg[9 + (i*11)] = (uint32_t)data->S32PositionDTerm;
		msg[10 + (i*11)] = (uint32_t)data->S32PositionIAccumulator;

	}
    write(DiagnosticsSendSocket, msg, sizeof(msg));

    return;
}

int ReceiveManual(ManualData * data) {

uint16_t msg[MSGU16_LEN];

	if(recv(ManualReceiveSocket, msg, sizeof(msg), MSG_DONTWAIT) < 0){
		return -1;
	}
	else {
		//Parse msg into fields of data struct
		data->Enable = msg[ENABLE_MSG];
		data->Mode = msg[MODE_MSG];
		data->Aux = msg[AUX_MSG];

		data->latPosition = msg[LAT_MSG];
		data->vertPosition = msg[VERT_MSG];
		data->Axis3Position = msg[AXIS3_MSG];
		data->Axis4Position = msg[AXIS4_MSG];

		return 1;
    }
}

int ReceiveDiagnostics(Diagnostics * lat, Diagnostics * vert) {

int i;
uint32_t msg[22];
Diagnostics * data;

	if(recv(DiagnosticsReceiveSocket, msg, sizeof(msg), MSG_DONTWAIT) < 0){
		return -1;
	}
	for(i = 0; i < 2; ++i) {
		if(i == 0)
			data = lat;
		else
			data = vert;

		data->U16FeedbackADC = (uint16_t)msg[0 + (i*11)];
		data->U16FeedbackADCPrevious = (uint16_t)msg[1 + (i*11)];
		data->S16OutputCommand = (int16_t)msg[2 + (i*11)];
		data->U16PositionDesired = (uint16_t)msg[3 + (i*11)];
		data->U16PositionActual = (uint16_t)msg[4 + (i*11)];
		data->S16PositionError = (int16_t)msg[5 + (i*11)];
		data->S16PositionErrorPrevious = (int16_t)msg[6 + (i*11)];
		data->S32PositionPTerm = (int32_t)msg[7 + (i*11)];
		data->S32PositionITerm = (int32_t)msg[8 + (i*11)];
		data->S32PositionDTerm = (int32_t)msg[9 + (i*11)];
		data->S32PositionIAccumulator = (int32_t)msg[10 + (i*11)];

	}

	return 0;
}

void ReceiveNeutral(Neutral * data) {

uint8_t msg[sizeof(*data)];

    recv(NeutralReceiveSocket, msg, sizeof(msg), MSG_DONTWAIT);

	data->latNeutral = msg[LAT_AXIS];
	data->vertNeutral = msg[VERT_AXIS];

    return;
}

void ReceiveSLA(SLAData * data) {

uint16_t msg[100];

	recv(NeutralReceiveSocket, msg, sizeof(msg), MSG_DONTWAIT);

	data->Column = msg[SLA_COLUMN_OFFSET];
	data->Row = msg[SLA_ROW_OFFSET];

	return;
}
