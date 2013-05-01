/*! \file fc_net.h
 *
 * Define the Flight Computer network information
 *
 */

#ifndef _FC_NET_H
#define _FC_NET_H


#if LWIP_NETCONN
	#include "lwip/ip_addr.h"
	#define         IP_PSAS_FC(p)           IP4_ADDR(p, 192, 168, 0, 91)
#endif
#define         FC_LISTEN_PORT_IMU_A     36000   // different ports for different sensors?
#define         FC_LISTEN_PORT_ROLL_CTL  36002

typedef enum psas_sensor_id {
	ID_ADIS    = 0,
	ID_MPU9150,
	ID_MPL3115A2
} PSAS_Sensor_id;


typedef struct psas_data_header {
	PSAS_Sensor_id       id;
	uint16_t             data_length;
} PSAS_data_header;


#endif
