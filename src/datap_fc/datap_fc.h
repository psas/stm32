/*! \file datap_fc.h
 */

#ifndef _DATAP_FC_H
#define _DATAP_FC_H

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "fc_net.h"

#define         NETBUFLEN               512
#define         NPACK                   100
#define         PORT_STRING_LEN         6

#define         NODE_OUT                35000
#define         NODE_IN                 35003

typedef enum thd_index {
	SENSOR_LISTENER,
	CONTROL_LISTENER
} Thdindex;

typedef struct Ports {
	unsigned int       thread_id;
	char               host_listen_port[PORT_STRING_LEN];
    struct sockaddr_in si_me;
    struct sockaddr_in si_sensor;
    struct sockaddr_in si_control;
} Ports;


#endif



