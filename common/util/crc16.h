/*! \file crc16.h - CRC-16 routine
 *
 * Implements the standard CRC-16:
 *   Width 16
 *   Poly  0x8005 (x^16 + x^15 + x^2 + 1)
 *   Init  0
 *
 * Copyright (c) 2005 Ben Gardner <bgardner@wabtec.com>
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
 */

/*! PSAS stm32.git :
 *  This code was adapted from the crc16 implementation in the linux
 *  kernel source.
 *  REF:    url = https://github.com/torvalds/linux.git
 *                   ./include/linux/crc16.h
 *                   ./lib/crc16.c
 *  Changes: include stdint library
 *           change u16 to uint16_t
 *           change u8  to uint8_t
 */

#ifndef CRC16_H_
#define CRC16_H_

#include <stdint.h>

extern uint16_t const crc16_table[256];

extern uint16_t crc16(uint16_t crc, const uint8_t *buffer, size_t len);

static inline uint16_t crc16_byte(uint16_t crc, const uint8_t data)
{
	return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}

#endif

