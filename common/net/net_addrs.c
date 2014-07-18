#include "lwipthread.h"

#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

/*
 * HELPER TYPES AND MACROS
 * ====================== *****************************************************
 */

/* htons() in macro form because lwip doesn't declare it as a macro (ugh) */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HTONS(n) __builtin_bswap16(n)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define HTONS(n) (n)
#endif

/* htonl() in macro form because lwip doesn't declare it as a macro (ugh) */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HTONL(n) __builtin_bswap32(n)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define HTONL(n) (n)
#endif

/** Set an IP address given by the four byte-parts */
#define IPv4(a,b,c,d) \
        HTONL( \
            ((uint32_t)((a) & 0xff) << 24) | \
            ((uint32_t)((b) & 0xff) << 16) | \
            ((uint32_t)((c) & 0xff) << 8)  | \
             (uint32_t)((d) & 0xff) \
        )

/* Returns a pointer to a filled anonymous struct sockaddr_in
 * suitable for direct use in bind() and connect()
 */
#define make_addr(addr, port) \
(const struct sockaddr *) &(const struct sockaddr_in){ \
        .sin_family = AF_INET, \
        .sin_port = HTONS((port)), \
        .sin_addr = { (addr) } \
}

/* Returns a pointer to a filled anonymous struct lwipthread_opts */
#define make_lwipopts(mac, ip, nmask, gway) \
&(struct lwipthread_opts){ \
        .macaddress = (mac), \
        .address = (ip), \
        .netmask = (nmask), \
        .gateway = (gway) \
}

/*
 * ADDRESS DEFINITIONS
 * =================== ********************************************************
 */

/* RNet Common */
#define NETMASK IPv4(255, 255, 255, 0)
#define GATEWAY IPv4(10,  10, 10, 1)

/* Flight Computer */
#define FC_IP IPv4(10, 10, 10, 10)
#define FC_LISTEN_PORT 36000 // FC device listener
const struct sockaddr * FC_ADDR = make_addr(FC_IP, FC_LISTEN_PORT);

/* Sensor Node */
#define SENSOR_IP IPv4(10, 10, 10, 20)
#define SENSOR_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}
#define ADIS_PORT 35020 // ADIS16405
#define MPU_PORT 35002  // MPU1950
#define MPL_PORT 35010  // MPL3115A2

struct lwipthread_opts * SENSOR_LWIP = make_lwipopts(SENSOR_MAC, SENSOR_IP, NETMASK, GATEWAY);
const struct sockaddr *ADIS_ADDR = make_addr(SENSOR_IP, ADIS_PORT);
const struct sockaddr *MPU_ADDR = make_addr(SENSOR_IP, MPU_PORT);
const struct sockaddr *MPL_ADDR = make_addr(SENSOR_IP, MPL_PORT);

/* Roll Control */
#define ROLL_IP IPv4(10, 10, 10, 30)
#define ROLL_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xbb}
#define ROLL_PORT 35003    // Servo control

struct lwipthread_opts * ROLL_LWIP = make_lwipopts(ROLL_MAC, ROLL_IP, NETMASK, GATEWAY);
const struct sockaddr * ROLL_ADDR = make_addr(ROLL_IP, ROLL_PORT);

/* Rocket Net Hub */
#define RNH_IP IPv4(10, 10, 10, 5)
#define RNH_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}
#define RNH_BATTERY 36101 // Battery data
#define RNH_PORT 36102    // Port data
#define RNH_ALARM 36103   // Battery alarm
#define RNH_UMBDET 36104  // Umbilical detect

struct lwipthread_opts * RNH_LWIP = make_lwipopts(RNH_MAC, RNH_IP, NETMASK, GATEWAY);
const struct sockaddr * RNH_BATTERY_ADDR = make_addr(RNH_IP, RNH_BATTERY);
const struct sockaddr * RNH_PORT_ADDR = make_addr(RNH_IP, RNH_PORT);
const struct sockaddr * RNH_ALARM_ADDR = make_addr(RNH_IP, RNH_ALARM);
const struct sockaddr * RNH_UMBDET_ADDR = make_addr(RNH_IP, RNH_UMBDET);

/* Rocket Tracks Controller */
#define RTX_IP IPv4(10, 0, 0, 40)
#define RTX_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xdd}
#define RTX_MANUAL 36200  // Manual Control listener
#define RTX_NEUTRAL 36201 // Axis Neutral data
#define RTX_FROMSLA 36202 // Sightline listener
#define RTX_DIAG 36205 // Axis Diagnostic data

struct lwipthread_opts * RTX_LWIP = make_lwipopts(RTX_MAC, RTX_IP, NETMASK, IPv4(10,0,0,1));
const struct sockaddr * RTX_MANUAL_ADDR = make_addr(RTX_IP, RTX_MANUAL);
const struct sockaddr * RTX_NEUTRAL_ADDR = make_addr(RTX_IP, RTX_NEUTRAL);
const struct sockaddr * RTX_FROMSLA_ADDR = make_addr(RTX_IP, RTX_FROMSLA);
const struct sockaddr * RTX_DIAG_ADDR = make_addr(RTX_IP, RTX_DIAG);

/* Rocket Tracks Manual Control Box */
#define RTXMAN_IP IPv4(10, 0, 0, 45)
#define RTXMAN_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xee}
#define RTXMAN_OUT 36203     // Manual Control data
#define RTXMAN_NEUTRAL 36204 // Axis Neutral listener
#define RTXMAN_DIAG 36206 // Axis Neutral listener

struct lwipthread_opts * RTXMAN_LWIP = make_lwipopts(RTXMAN_MAC, RTXMAN_IP, NETMASK, IPv4(10,0,0,1));
const struct sockaddr * RTXMAN_OUT_ADDR = make_addr(RTXMAN_IP, RTXMAN_OUT);
const struct sockaddr * RTXMAN_NEUTRAL_ADDR = make_addr(RTXMAN_IP, RTXMAN_NEUTRAL);
const struct sockaddr * RTXMAN_DIAG_ADDR = make_addr(RTXMAN_IP, RTXMAN_DIAG);


/* GPS frontend */
#define GPS_IP IPv4(10, 10, 10, 40)
#define GPS_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xff}
#define GPS_OUT 35050
#define GPS_COTS 35051

struct lwipthread_opts * GPS_LWIP = make_lwipopts(GPS_MAC, GPS_IP, NETMASK, GATEWAY);
const struct sockaddr * GPS_OUT_ADDR = make_addr(GPS_IP, GPS_OUT);
const struct sockaddr * GPS_COTS_ADDR = make_addr(GPS_IP, GPS_COTS);
