/*! \file device_net.h
 *
 * Network information for devices connected to the Flight Computer network
 *
 */

#ifndef _DEVICE_NET_H
#define _DEVICE_NET_H

#include "lwip/ip_addr.h"

#define         IMU_A_IP_ADDR(p)        IP4_ADDR(p, 192, 168, 0, 196);
#define         IMU_A_LISTEN_PORT       35001
#define         IMU_A_TX_PORT           35002

#define         ROLL_CTL_IP_ADDR(p)     IP4_ADDR(p, 192, 168, 0, 197);
#define         ROLL_CTL_LISTEN_PORT    35003
#define         ROLL_CTL_TX_PORT        35004

#endif
