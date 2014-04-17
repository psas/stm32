/*
 * Provides filled address structures suitable for direct use in their
 * respective APIs (i.e. struct sockaddr pointers for sockets). Some structs
 * aren't declared as constant because the APIs don't expect them to be, but
 * please treat them as if they were.
 */

#ifndef NET_ADDRS_H_
#define NET_ADDRS_H_

#include "lwipthread.h"
#include "lwip/ip_addr.h"

/* Flight computer listen address */
extern const struct sockaddr * FC_ADDR;

/* Sensor Node */
extern struct lwipthread_opts * SENSOR_LWIP;
extern const struct sockaddr * ADIS_ADDR; // ADIS16405
extern const struct sockaddr * MPU_ADDR;  // MPU1950
extern const struct sockaddr * MPL_ADDR;  // MPL3115A2

/* Servo Node */
extern struct lwipthread_opts * ROLL_LWIP;
extern const struct sockaddr * ROLL_ADDR;    // Servo control
extern const struct sockaddr * TEATHER_ADDR; // Launch Detect

/* Rocket Net Hub */
extern struct lwipthread_opts * RNH_LWIP;
extern const struct sockaddr * RNH_RCI_ADDR;     // RCI listener
extern const struct sockaddr * RNH_BATTERY_ADDR; // Battery data
extern const struct sockaddr * RNH_PORT_ADDR;    // Port data

#endif /* NET_ADDRS_H_ */
