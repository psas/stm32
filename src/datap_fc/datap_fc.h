/*! \file datap_fc.h
 */

#ifndef _DATAP_FC_H
#define _DATAP_FC_H

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define         NETBUFLEN       512
#define         NPACK           100

#define         PORT_OUT        35000
#define         PORT_IN         35003
#define         SENSOR_IP       "192.168.0.196"
#define         CONTROL_IP      "192.168.0.197"

typedef enum boards {
	SENSOR_BOARD,
	CONTROL_BOARD
} Board;

typedef struct Ports {
	unsigned int       thread_id;
    struct sockaddr_in si_me;
    struct sockaddr_in si_other;
} Ports;


#endif



