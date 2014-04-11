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

#define MANUAL_TX_PORT              		35000
#define SLA_TX_PORT              			35006
#define MANUAL_RX_PORT         	   			35003
#define SLA_RX_PORT      	   	   			35009

#define RTX_CONTROLLER_RX_THD_PRIORITY             (LOWPRIO)

#define IP_HOST                             "192.168.0.91"
#define IP_SLA	                            "192.168.0.196"
#define IP_MANUAL                           "192.168.0.123"

#define MANUAL_REMOTE_MESSAGE_SIZE			11
#define SLA_MESSAGE_SIZE					2


typedef struct {

	char Enable;
	char Mode;
	char Aux;
	uint16_t latPosition;
	uint16_t vertPosition;
	uint16_t Axis3Position;
	uint16_t Axis4Position;
} ManualData;

typedef struct {

	char latNeutral;
	char vertNeutral;
} Neutral;

typedef struct {
	//TODO Robert use this for parsed data received from SLA
} SLAData;

void SendSLA(int Command);
void ReceiveSLA(SLAData data);
void ReceiveManual(ManualData data);
void SendNeutral(Neutral data);

extern WORKING_AREA(wa_rtx_controller_receive_thread, RTX_RECEIVE_THREAD_STACK_SIZE);

#ifdef __cplusplus
extern "C" {
#endif
  msg_t rtx_controller_receive_thread(void *p);

#ifdef __cplusplus
}
#endif

#endif /* ENET_API_H_ */
