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
#define         NPACK                   1000
#define         PORT_STRING_LEN         6


typedef enum thd_index {
	SENSOR_LISTENER,
	CONTROL_LISTENER
} Thdindex;

typedef struct Ports {
	unsigned int       thread_id;
	char               host_listen_port[PORT_STRING_LEN];
	char               client_addr[INET6_ADDRSTRLEN];
	char               client_port[PORT_STRING_LEN];
} Ports;

/*! \typedef Structure for accelerometer data
 *
 *
 */
typedef struct mpu9150_accel_data {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} MPU9150_accel_data;

/*! \typedef Structure for gyroscope data
 *
 *
 */
typedef struct mpu9150_gyro_data {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} MPU9150_gyro_data;

/*! \typedef Read Data from mpu9150
 *
 */
typedef struct mpu9150_read_data {
	MPU9150_gyro_data     gyro_xyz;
	MPU9150_accel_data    accel_xyz;
	int16_t               celsius;
} MPU9150_read_data;


#endif



