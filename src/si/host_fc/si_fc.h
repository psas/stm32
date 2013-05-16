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
#define         NPACK                   100
#define         PORT_STRING_LEN         6


typedef enum thd_index {
	MPU_LISTENER,
	ADIS_LISTENER
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

typedef     uint16_t                          adis_reg_data;

/*! \typedef
 * Burst data collection
 *
 */
typedef struct {
	adis_reg_data      adis_supply_out; //  Power supply measurement
	adis_reg_data      adis_xgyro_out;  //  X-axis gyroscope output
	adis_reg_data      adis_ygyro_out;  //  Y-axis gyroscope output
	adis_reg_data      adis_zgyro_out;  //  Z-axis gyroscope output
	adis_reg_data      adis_xaccl_out;  //  X-axis accelerometer output
	adis_reg_data      adis_yaccl_out;  //  Y-axis accelerometer output
	adis_reg_data      adis_zaccl_out;  //  Z-axis accelerometer output
	adis_reg_data      adis_xmagn_out;  //  X-axis magnetometer measurement
	adis_reg_data      adis_ymagn_out;  //  Y-axis magnetometer measurement
	adis_reg_data      adis_zmagn_out;  //  Z-axis magnetometer measurement
	adis_reg_data      adis_temp_out;   //  Temperature output
	adis_reg_data      adis_aux_adc;    //  Auxiliary ADC measurement
} ADIS16405_burst_data;

#endif



