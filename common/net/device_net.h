/*! \file device_net.h
 *
 * Network information for devices connected to the Flight Computer network
 *
 */

#ifndef _DEVICE_NET_H
#define _DEVICE_NET_H

#include "fc_net.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#define FC_IP "10.0.0.10"

#define         IMU_A_IP_ADDR_STRING        "10.0.0.20"
#define         IMU_A_LISTEN_PORT           35001
#define         IMU_A_TX_PORT               35002

// Sensor Node
#define SENSOR_IP "10.0.0.20"

// ADIS
#define ADIS_RX_PORT 35020

#define         IMU_A_MAC_ADDRESS           {0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}

#define         ROLL_CTL_IP_ADDR_STRING     "192.168.0.197"
#define         ROLL_CTL_LISTEN_PORT        35003
#define         ROLL_CTL_TX_PORT            35004
#define         ROLL_CTL_MAC_ADDRESS        {0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}

struct sockaddr * make_sockaddr(struct sockaddr_in * addr, const char * ip, int port);
int get_udp_socket(struct sockaddr *addr);

extern struct sockaddr_in adis_out;
#define ADIS_OUT_ADDR make_sockaddr(&adis_out, SENSOR_IP, ADIS_RX_PORT)

extern struct sockaddr_in adis_in;
#define ADIS_IN_ADDR make_sockaddr(&adis_in, FC_IP,  FC_LISTEN_PORT_IMU_A)

struct lwipthread_opts * get_adis_addr(void);

#endif


