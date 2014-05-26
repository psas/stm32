//******************************************************************************
// enet_api.h
//
//  Created on: Mar 26, 2014
//      Author: Rob Gaskell		PSU ID# 914464617
// 	   Project: RocketTracks
//******************************************************************************

#ifndef ENET_API_H_
#define ENET_API_H_

#define RTX_RECEIVE_THREAD_STACK_SIZE   	512

#define SLA_TX_PORT              	35005
#define SLA_RX_PORT					35006
#define NEUTRAL_TX_PORT       		35008
#define NEUTRAL_RX_PORT				35009
#define MANUAL_TX_PORT              35011
#define MANUAL_RX_PORT				35012

#define RTX_CONTROLLER_RX_THD_PRIORITY             (LOWPRIO)

#define IP_RTX                             	"192.168.0.91"
#define IP_SLA	                            "192.168.0.196"
#define IP_MANUAL                           "192.168.0.123"

#define MANUAL_REMOTE_MESSAGE_SIZE			11
#define SLA_MESSAGE_SIZE					2

#define SLA_COLUMN_OFFSET					7
#define SLA_ROW_OFFSET						9

extern int RTxtoManualSendSocket;
extern int ManualtoRTxSendSocket;

extern int RTxfromSLAReceiveSocket;
extern int ManualReceiveSocket;
extern int NeutralReceiveSocket;

typedef struct {
	uint8_t Enable;
	uint8_t Mode;
	uint8_t Aux;
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

void SendRTxtoManualSocket(void);
void ReceiveRTxfromSLASocket(void);
void ReceiveRTxfromManualSocket(void);

void SendManualtoRTxSocket(void);
void ReceiveManualfromRTxSocket(void);

void SendSLA(int Command);
void ReceiveSLA(SLAData * data);
int ReceiveManual(ManualData * data);
void SendManual(ManualData * data);
void ReceiveNeutral(Neutral * data);
void SendNeutral(Neutral * data);

#endif /* ENET_API_H_ */
