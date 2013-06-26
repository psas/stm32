/*! \file datap_fc.h
*/

#ifndef _DATAP_FC_H
#define _DATAP_FC_H

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "fc_net.h"

#define         NPACK                   100
#define         PORT_STRING_LEN         6

typedef enum sensorID {
    UNKNOWN_SENSOR,
    ADIS,
    MPU,
    MPL
} SensorID;

typedef enum thd_index {
    MPU_LISTENER,
    MPL_LISTENER,
    ADIS_LISTENER
} Thdindex;

typedef struct Ports {
    unsigned int       thread_id;
    char               host_listen_port[PORT_STRING_LEN];
    char               client_addr[INET6_ADDRSTRLEN];
    char               client_port[PORT_STRING_LEN];
} Ports;

typedef struct Usertalk {
	unsigned int       thread_id;
	char               host_listen_port[PORT_STRING_LEN];
	char               client_a_addr[INET6_ADDRSTRLEN];
	char               client_a_port[PORT_STRING_LEN];
	char               client_b_addr[INET6_ADDRSTRLEN];
	char               client_b_port[PORT_STRING_LEN];
} Usertalk;


/*! \typedef Structure for accelerometer data
 *
 *
 */
struct MPU9150_accel_data {
	uint16_t x;
	uint16_t y;
	uint16_t z;
}  __attribute__((packed)) ;
typedef struct MPU9150_accel_data MPU9150_accel_data;

/*! \typedef Structure for gyroscope data
 *
 *
 */
struct MPU9150_gyro_data {
	uint16_t x;
	uint16_t y;
	uint16_t z;
}  __attribute__((packed)) ;
typedef struct MPU9150_gyro_data MPU9150_gyro_data;

/*! \typedef Read Data from mpu9150
 *
 */
struct MPU9150_read_data {
	MPU9150_gyro_data     gyro_xyz;
	MPU9150_accel_data    accel_xyz;
	int16_t               celsius;
}  __attribute__((packed)) ;
typedef struct MPU9150_read_data MPU9150_read_data;


struct MPU_packet {
    char                ID[4];
    uint8_t             timestamp[16];
    uint16_t            data_length;
    MPU9150_read_data   data;
}  __attribute__((packed)) ;
typedef struct MPU_packet MPU_packet;



typedef     uint32_t                                mpl3115a2_pressure_data;
typedef     uint32_t                                mpl3115a2_temperature_data;

/*! \typedef  mpl3115a2 data
 *
 */
struct MPL3115A2_read_data {
    mpl3115a2_pressure_data         mpu_pressure;
    mpl3115a2_temperature_data      mpu_temperature;
} __attribute__((packed)) ;
typedef struct MPL3115A2_read_data MPL3115A2_read_data;


struct MPL_packet {
    char                ID[4];
    uint8_t             timestamp[16];
    uint16_t            data_length;
    MPL3115A2_read_data data;
}  __attribute__((packed)) ;
typedef struct MPL_packet MPL_packet;


typedef     uint16_t                          adis_reg_data;

/*! \typedef
 * Burst data collection
 */
struct ADIS16405_burst_data {
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
} __attribute__((packed));
typedef struct ADIS16405_burst_data ADIS16405_burst_data;

struct ADIS_packet {
    char                 ID[4];
    uint8_t              timestamp[16];
    uint16_t             data_length;
    ADIS16405_burst_data data;
} __attribute__((packed));
typedef struct ADIS_packet ADIS_packet;

#endif



