/*! \file psas_packet.h
 *
 * \sa http://psas.pdx.edu/avionics/av3-data-protocol/
 */

#ifndef _PSAS_PACKET_H
#define _PSAS_PACKET_H

#include <stdint.h>


/*!
 * \addtogroup psaspacket
 * @{
 */


struct RC_LAUNCH_DETECT_STRUCT_TYPE {
    // Launch Detect 1 when launch detected. 0 when no launch detected.
    uint8_t launch_detect;
} __attribute__((packed));
typedef struct RC_LAUNCH_DETECT_STRUCT_TYPE RC_LAUNCH_DETECT_STRUCT_TYPE;



/*! \warning The structure from the RC module on the Flight computer
 * is not packed. Current plans are to pass a byte array from the FC
 * to the RC board. This structure is here for use and reference locally
 * but do not memcpy the network data into it, it may not work.
 */


struct RC_OUTPUT_STRUCT_TYPE {
    // Servo ON-Time in milliseconds x 2^14
    // Example: 1.5 msec = 1.5 x 2^14 = 24576
    uint16_t u16ServoPulseWidthBin14;

    // Disable servo (turn off PWM) when this flag is not 0
    uint8_t u8ServoDisableFlag;
} __attribute__((packed));
typedef struct RC_OUTPUT_STRUCT_TYPE RC_OUTPUT_STRUCT_TYPE;

#endif
//! @}


