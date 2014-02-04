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

/* Creates a filled local struct sockaddr_in and a global pointer to it
 * suitable for direct use in bind() and connect()
 */
#define make_addr(name, ip, port)                                             \
static struct sockaddr_in (name ## _tmp_) = {                                 \
    .sin_family = AF_INET,                                                    \
    .sin_port = HTONS((port)),                                                \
    .sin_addr.s_addr = (ip)                                                   \
};                                                                            \
struct sockaddr * (name) = (struct sockaddr *)&(name ## _tmp_)

/* Creates a filled local struct lwipthread_opts and a global pointer to it */
#define make_lwipopts(name, mac, ip, nmask, gway)                             \
static struct lwipthread_opts (name ## _tmp_) = {                             \
    .macaddress = (mac),                                                      \
    .address = (ip),                                                          \
    .netmask = (nmask),                                                       \
    .gateway = (gway),                                                        \
};                                                                            \
struct lwipthread_opts * (name) = &(name ## _tmp_)

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

make_addr(FC_ADDR, FC_IP, FC_LISTEN_PORT);

/* Sensor Node */
#define SENSOR_IP IPv4(10, 0, 0, 20)
#define SENSOR_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0x11}
#define ADIS_PORT 35020 // ADIS16405
#define MPU_PORT 35002  // MPU1950
#define MPL_PORT 35010  // MPL3115A2

make_lwipopts(SENSOR_LWIP, SENSOR_MAC, SENSOR_IP, NETMASK, GATEWAY);
make_addr(ADIS_ADDR, SENSOR_IP, ADIS_PORT);
make_addr(MPU_ADDR, SENSOR_IP, MPU_PORT);
make_addr(MPL_ADDR, SENSOR_IP, MPL_PORT);

/* Roll Control */
#define ROLL_IP IPv4(10, 0, 0, 30)
#define ROLL_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xbb}
#define ROLL_PORT 35003    // Servo control
#define TEATHER_PORT 35004 // Launch detect

make_lwipopts(ROLL_LWIP, ROLL_MAC, ROLL_IP, NETMASK, GATEWAY);
make_addr(ROLL_ADDR, ROLL_IP, ROLL_PORT);
make_addr(TEATHER_ADDR, ROLL_IP, TEATHER_PORT);

/* Rocket Net Hub */
#define RNH_IP IPv4(10, 0, 0, 5)
#define RNH_MAC (uint8_t[6]){0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}
#define RNH_LISTEN 36100 //?
#define RNH_SEND 36101     //???

make_lwipopts(RNH_LWIP, RNH_MAC, RNH_IP, NETMASK, GATEWAY);
make_addr(RNH_LISTEN_ADDR, RNH_IP, RNH_LISTEN);
make_addr(RNH_SEND_ADDR, RNH_IP, RNH_SEND);

