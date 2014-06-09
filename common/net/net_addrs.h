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

/* Rocket Tracks Controller */
extern struct lwipthread_opts * RTX_LWIP;
extern const struct sockaddr * RTX_MANUAL_ADDR;  // Manual Control listener
extern const struct sockaddr * RTX_NEUTRAL_ADDR; // Axis Neutral data
extern const struct sockaddr * RTX_FROMSLA_ADDR; // Sightline listener
extern const struct sockaddr * RTX_DIAG_ADDR;	// Axis Diagnostic data

/* Rocket Tracks Manual Control Box */
extern struct lwipthread_opts * RTXMAN_LWIP;
extern const struct sockaddr * RTXMAN_OUT_ADDR;     // Manual Control data
extern const struct sockaddr * RTXMAN_NEUTRAL_ADDR; // Axis Neutral listener
extern const struct sockaddr * RTXMAN_DIAG_ADDR;	// Axis Diagnostic listener

/* GPS frontend */
extern struct lwipthread_opts * GPS_LWIP;
extern struct sockaddr * GPS_OUT; // Data's gotta be flowing out, right?

#endif /* NET_ADDRS_H_ */
