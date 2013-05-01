/*! \file datap_fc.h
 */

#ifndef _DATAP_FC_H
#define _DATAP_FC_H

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "fc_net.h"

#define         MAXBUFLEN               512
#define         NETBUFLEN               512
#define         NPACK                   10
#define         PORT_STRING_LEN         6

typedef enum thd_index {
	CONTROL_LISTENER,
	SENSOR_LISTENER
} Thdindex;

typedef struct Ports {
	unsigned int       thread_id;
	char               host_listen_port[PORT_STRING_LEN];
	char               client_addr[INET6_ADDRSTRLEN];
	char               client_port[PORT_STRING_LEN];
} Ports;


#endif



