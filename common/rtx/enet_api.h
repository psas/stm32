//******************************************************************************
// enet_api.h
//
//  Created on: Mar 26, 2014
//      Author: Rob Gaskell		PSU ID# 914464617
// 	   Project: RocketTracks
//******************************************************************************

#ifndef ENET_API_H_
#define ENET_API_H_

#include "rocket_tracks.h"

extern int RTxtoManualSendSocket;
extern int ManualtoRTxSendSocket;

extern int RTxfromSLAReceiveSocket;
extern int ManualReceiveSocket;
extern int NeutralReceiveSocket;

typedef struct {
	uint16_t Enable;
	uint16_t Mode;
	uint16_t Aux;
	uint16_t latPosition;
	uint16_t vertPosition;
	uint16_t Axis3Position;
	uint16_t Axis4Position;
} ManualData;

typedef struct {
	uint8_t latNeutral;
	uint8_t vertNeutral;
} Neutral;

typedef struct {
	uint16_t Column;
	uint16_t Row;
} SLAData;

typedef struct {

	axissample_t U16FeedbackADC;
	axissample_t U16FeedbackADCPrevious;

	int16_t S16OutputCommand;				//PWM on-time

	uint16_t U16PositionDesired;
	uint16_t U16PositionActual;
	int16_t S16PositionError;

	uint16_t S16PositionErrorPrevious;

	int32_t S32PositionPTerm;				//Proportional Feedback
	int32_t S32PositionITerm;				//Integral Feedback
	int32_t S32PositionDTerm;				//Derivative Feedback
	int32_t S32PositionIAccumulator;

} Diagnostics;

void SendRTxtoManualSocket(void);
void ReceiveRTxfromSLASocket(void);
void ReceiveRTxfromManualSocket(void);

void SendManualtoRTxSocket(void);
void ReceiveManualfromRTxSocket(void);

void SendDiagnosticsSocket(void);
void ReceiveDiagnosticsSocket(void);

void SendSLA(int Command);
int ReceiveSLA(SLAData * data);
int ReceiveManual(ManualData * data);
void SendManual(ManualData * data);
void ReceiveNeutral(Neutral * data);
void SendNeutral(Neutral * data);

void SendDiagnostics(Diagnostics * lat, Diagnostics * vert, uint16_t ref);
int ReceiveDiagnostics(Diagnostics * lat, Diagnostics * vert, uint16_t * ref);

#endif /* ENET_API_H_ */
