/*! \file time_net.h
 *
 * Network information for bulldada time control experiments
 *
 */

#ifndef _TIME_NET_H
#define _TIME_NET_H

#if LWIP_NETCONN
#include "lwip/ip_addr.h"
#endif

#if LWIP_NETCONN
#define         TIME_CLIENT_IP_ADDR(p)            IP4_ADDR(p, 10, 0, 0, 3);
#define         TIME_CLIENT_GATEWAY(p)            IP4_ADDR(p, 10, 0, 0, 1);
#define         TIME_CLIENT_NETMASK(p)            IP4_ADDR(p, 255, 255, 255, 0);
#define			 TIME_SERVER_IP_ADDR(p)				  IP4_ADDR(p, 10, 0, 0, 2);
#endif
#define         TIME_CLIENT_IP_ADDR_STRING        "10.0.0.3"
#define         TIME_CLIENT_LISTEN_PORT           31338
#define         TIME_CLIENT_TX_PORT               31337
#define         TIME_CLIENT_MAC_ADDRESS           {0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}
#endif

