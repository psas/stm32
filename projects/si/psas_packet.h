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

struct ADIS_packet {
    char                 ID[4];
    uint8_t              timestamp[6];
    uint16_t             data_length;
    ADIS16405_burst_data data;
} __attribute__((packed));
typedef struct ADIS_packet ADIS_packet;

struct MPU_packet {
    char                ID[4];
    uint8_t             timestamp[6];
    uint16_t            data_length;
    MPU9150_read_data   data;
}  __attribute__((packed)) ;
typedef struct MPU_packet MPU_packet;

struct MPL_packet {
    char                ID[4];
    uint8_t             timestamp[6];
    uint16_t            data_length;
    MPL3115A2_read_data data;
}  __attribute__((packed)) ;
typedef struct MPL_packet MPL_packet;


#endif
//! @}


