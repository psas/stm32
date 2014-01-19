/*! \file device_net.h
 *
 * Network information for devices connected to the Flight Computer network
 *
 */

#ifndef _DEVICE_NET_H
#define _DEVICE_NET_H

#include "net_addrs.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#define         IMU_A_MAC_ADDRESS           {0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}
#define         ROLL_CTL_MAC_ADDRESS        {0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}

struct sockaddr * make_sockaddr(struct sockaddr_in * addr, const char * ip, int port);
int get_udp_socket(struct sockaddr *addr);

extern struct sockaddr_in adis_out;
#define ADIS_ADDR make_sockaddr(&adis_out, SENSOR_IP, ADIS_RX_PORT)

extern struct sockaddr_in fc_listen;
#define FC_LISTEN_ADDR make_sockaddr(&fc_listen, FC_IP,  FC_LISTEN_PORT)

struct lwipthread_opts * get_adis_addr(void);

#endif


