/*! \file psas_packet.h
 *
 * \sa http://psas.pdx.edu/avionics/av3-data-protocol/
 */

#ifndef _PSAS_PACKET_H
#define _PSAS_PACKET_H

#include "MPU9150.h"
#include "ADIS16405.h"
#include "MPL3115A2.h"

/*!
 * \addtogroup psaspacket
 * @{
 */

struct RC_OUTPUT_STRUCT_TYPE {
    // Servo ON-Time in milliseconds x 2^14
    // Example: 1.5 msec = 1.5 x 2^14 = 24576
    uint16_t u16ServoPulseWidthBin14;

    // Disable servo (turn off PWM) when this flag is not 0
    uint8_t u8ServoDisableFlag;
} __attribute__((packed)) ;

typedef struct RC_OUTPUT_STRUCT_TYPE RC_OUTPUT_STRUCT_TYPE;

#endif
//! @}


