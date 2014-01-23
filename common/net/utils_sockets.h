/*
 * Socket and address related utilities
 *
 */

#ifndef _DEVICE_NET_H
#define _DEVICE_NET_H

#include "lwipthread.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#include "net_addrs.h"

/* Convenience functions for filling out structs and setting up sockets */
void set_lwipthread_opts(struct lwipthread_opts * ip_opts,
    const char * ip, const char * netmask, const char * gateway, uint8_t * mac);
void set_sockaddr(struct sockaddr_in * addr, const char * ip, int port);
int get_udp_socket(struct sockaddr *addr);


/*FIXME: Ideally the following macros would be const structs pre-filled with
 *       data but so far I've not come up with a good way to make it easy to
 *       maintain and be sync-able with net_addrs.h
 */

/* MAC addresses */
/* (type[]){initializer} is apparently valid C99 for an anonymous array. It's
 * dangerous though in that you can put whatever the hell you want in the
 * initializer and it wont warn about types or lengths or anything.
 * Adding the size seems to turn on some warnings.
 */
#define IMU_MAC_ADDRESS      (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}
#define ROLL_CTL_MAC_ADDRESS (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}

/* lwip thread options */
#define SET_IMU_LWIPOPTS(lwipopt_ptr) set_lwipthread_opts((lwipopt_ptr), \
        SENSOR_IP, "255.0.0.0", "10.0.0.1", IMU_MAC_ADDRESS)

/* socket addresses */
#define SET_ADIS_ADDR(sockaddr_in_ptr) set_sockaddr((sockaddr_in_ptr), \
        SENSOR_IP, ADIS_RX_PORT)

#define SET_FC_LISTEN_ADDR(sockaddr_in_ptr) set_sockaddr((sockaddr_in_ptr), \
        FC_IP,  FC_LISTEN_PORT)

#endif


