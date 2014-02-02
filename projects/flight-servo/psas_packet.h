#ifndef _PSAS_PACKET_H
#define _PSAS_PACKET_H


/*
 * Includes
 * ======== ********************************************************************
 */

#include <stdint.h>


/*
 * Type Definitions
 * ================ ************************************************************
 */

typedef struct {
    uint8_t launch_detect; // 1 when launch detected. 0 when no launch detected.
} __attribute__((packed)) LaunchDetect;


/*! \warning The structure from the RC module on the Flight computer
 * is not packed. Current plans are to pass a byte array from the FC
 * to the RC board. This structure is here for use and reference locally
 * but do not memcpy the network data into it, it may not work.
 */
typedef struct {
    uint16_t u16ServoPulseWidthBin14; // PWM on-time in milliseconds x 2^14
                                      // e.g. 1.5 msec = 1.5 x 2^14 = 24576
    uint8_t u8ServoDisableFlag;       // Disable servo (turn off PWM) when this flag is not 0
} __attribute__((packed)) RCOutput;


// boilerplate
#endif
