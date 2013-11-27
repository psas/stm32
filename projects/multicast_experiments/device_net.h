/*! \file device_net.h
 *
 * Network information for devices connected to the Flight Computer network
 *
 */

#ifndef _DEVICE_NET_H
#define _DEVICE_NET_H

#if LWIP_NETCONN
#include "lwip/ip_addr.h"
#endif


#if LWIP_NETCONN
#define         UNICAST_IP_ADDR(p)         IP4_ADDR(p, 10, 0, 0,   30    );
#define         UNICAST_GATEWAY(p)         IP4_ADDR(p, 10, 0, 0,   10    );
// #define         UNICAST_IP_ADDR(p)            IP4_ADDR(p, 192, 168, 0,   205    );
// #define         UNICAST_GATEWAY(p)            IP4_ADDR(p, 192, 168, 0,   91    );
#define         UNICAST_NETMASK(p)         IP4_ADDR(p, 255, 255, 255, 0  );
#endif
#define         UNICAST_IP_ADDR_STRING     "10.0.0.30"
//#define         UNICAST_IP_ADDR_STRING        "192.168.0.205"
#define         UNICAST_LISTEN_PORT        35003
#define         UNICAST_TX_PORT            35004
#define         UNICAST_MAC_ADDRESS        {0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}

#define			 MULTICAST_MOST_ADDRESS(p)		IP4_ADDR(p,224,0,1,129);
#define		    MULTICAST_PEER_DELAY_ADDRESS(p) IP4_ADDR(p,224,0,0,107);
#define			 MULTICAST_NETMASK(p)				IP4_ADDR(p,255,255,254,0);


#endif
