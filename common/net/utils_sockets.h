/*
 * Socket and address related utilities
 *
 */

#ifndef UTILS_SOCKETS_H_
#define UTILS_SOCKETS_H_

#include "lwipthread.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

/* Ethernet MTU in bytes - useful for creating UDP rx and tx buffers*/
#define ETH_MTU 1500

/* Fills out an lwipthread_opts struct */
void set_lwipthread_opts(struct lwipthread_opts * ip_opts,
    const char * ip, const char * netmask, const char * gateway, uint8_t * mac);

/* Starts the lwip core thread with ip_opts */
void lwipThreadStart(struct lwipthread_opts * ip_opts);


/* Fills out a sockaddr_in with AF_INET, ip, and port in network byteorder */
void set_sockaddr(struct sockaddr * addr, const char * ip, uint16_t port);

/* Returns an AF_INET UDP socket bound to addr, or less than 0 on failure */
int get_udp_socket(const struct sockaddr *addr);

#endif


