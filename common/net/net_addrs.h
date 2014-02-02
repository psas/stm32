/*
 * Provides filled address structures suitable for direct use in their
 * respective APIs (i.e. struct sockaddr pointers for sockets). Structs aren't
 * declared as constant because the APIs don't expect them to be, but please
 * treat them as if they were.
 */

#ifndef NET_ADDRS_H_
#define NET_ADDRS_H_

#include "lwipthread.h"
#include "lwip/ip_addr.h"

/* Flight computer listen address */
extern struct sockaddr * FC_ADDR;

/* Sensor Node */
extern struct lwipthread_opts * SENSOR_LWIP;
extern struct sockaddr * ADIS_ADDR; // ADIS16405
extern struct sockaddr * MPU_ADDR;  // MPU1950
extern struct sockaddr * MPL_ADDR;  // MPL3115A2

/* Servo Node */
extern struct lwipthread_opts * ROLL_LWIP;
extern struct sockaddr * ROLL_ADDR;    // Servo control
extern struct sockaddr * TEATHER_ADDR; // Launch Detect

/* Rocket Net Hub */
extern struct lwipthread_opts * RNH_LWIP;
extern struct sockaddr * RNH_LISTEN_ADDR;
extern struct sockaddr * RNH_SEND_ADDR;

#endif /* NET_ADDRS_H_ */
