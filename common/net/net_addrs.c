#include "lwipthread.h"

#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
/*
 * HELPER TYPES AND MACROS
 * ====================== *****************************************************
 */

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
/** Set an IP address given by the four byte-parts */
#define IPv4(a,b,c,d) \
        ((uint32_t)((a) & 0xff) << 24) | \
        ((uint32_t)((b) & 0xff) << 16) | \
        ((uint32_t)((c) & 0xff) << 8)  | \
         (uint32_t)((d) & 0xff))
#elif  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of htonl. */
#define IPv4(a,b,c,d) \
       ((u32_t)((d) & 0xff) << 24) | \
       ((u32_t)((c) & 0xff) << 16) | \
       ((u32_t)((b) & 0xff) << 8)  | \
        (u32_t)((a) & 0xff)
#endif

/* htons() in macro form because lwip doesn't declare it as a macro (ugh) */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HTONS(n) (uint16_t)((((uint16_t) (n)) << 8) | (((uint16_t) (n)) >> 8))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define HTONS(n) (n)
#endif

/* Returns a pointer to a filled anonymous struct sockaddr_in
 * suitable for direct use in bind() and connect()
 */
#define make_addr(port, addr) \
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
#define NETMASK IPv4(255, 0, 0, 0)
#define GATEWAY IPv4(10,  0, 0, 1)

/* Flight Computer */
#define FC_IP IPv4(10, 0, 0, 10)
#define FC_LISTEN_PORT 36000 // FC device listener
const struct sockaddr * FC_ADDR = make_addr(FC_IP, FC_LISTEN_PORT);

/* Sensor Node */
#define SENSOR_IP IPv4(10, 0, 0, 20)
#define SENSOR_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}
#define ADIS_PORT 35020 // ADIS16405
#define MPU_PORT 35002  // MPU1950
#define MPL_PORT 35010  // MPL3115A2

struct lwipthread_opts * SENSOR_LWIP = make_lwipopts(SENSOR_MAC, SENSOR_IP, NETMASK, GATEWAY);
const struct sockaddr *ADIS_ADDR = make_addr(SENSOR_IP, ADIS_PORT);
const struct sockaddr *MPU_ADDR = make_addr(SENSOR_IP, MPU_PORT);
const struct sockaddr *MPL_ADDR = make_addr(SENSOR_IP, MPL_PORT);

/* Roll Control */
#define ROLL_IP IPv4(10, 0, 0, 30)
#define ROLL_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xbb}
#define ROLL_PORT 35003    // Servo control
#define TEATHER_PORT 35004 // Launch detect

struct lwipthread_opts * ROLL_LWIP = make_lwipopts(ROLL_MAC, ROLL_IP, NETMASK, GATEWAY);
const struct sockaddr * ROLL_ADDR = make_addr(ROLL_IP, ROLL_PORT);
const struct sockaddr * TEATHER_ADDR = make_addr(ROLL_IP, TEATHER_PORT);

/* Rocket Net Hub */
#define RNH_IP IPv4(10, 0, 0, 5)
#define RNH_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}
#define RNH_LISTEN 36100 //?
#define RNH_SEND 36101     //???

struct lwipthread_opts * RNH_LWIP = make_lwipopts(RNH_MAC, RNH_IP, NETMASK, GATEWAY);
const struct sockaddr * RNH_LISTEN_ADDR = make_addr(RNH_IP, RNH_LISTEN);
const struct sockaddr * RNH_SEND_ADDR = make_addr(RNH_IP, RNH_SEND);

