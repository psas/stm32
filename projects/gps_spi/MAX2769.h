/* \file max2769.h */

/* Intended for use with ChibiOS RT */

#ifndef _MAX2769_H
#define _MAX2769_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ch.h"
#include "hal.h"

#include "utils_hal.h"

#if !defined(MAX2769_DEBUG) || defined(__DOXYGEN__)
#define         MAX2769_DEBUG                   0
#endif

/* MAX2769 Register addresses */
typedef enum
{
	// Name                               address         default value
	MAX2769_CONF1           = 0b0000,       // 0xA2919A3
	MAX2769_CONF2           = 0b0001,       // 0x0550288
	MAX2769_CONF3           = 0b0010,       // 0xEAFF1DC
	MAX2769_PLLCONF         = 0b0011,       // 0x9EC0008
	MAX2769_DIV             = 0b0100,       // 0x0C00080
	MAX2769_FDIV            = 0b0101,       // 0x8000070
	MAX2769_STRM            = 0b0110,       // 0x8000000
	MAX2769_CLK             = 0b0111,       // 0x10061B2
	MAX2769_TEST1           = 0b1000,       // 0x1E0F401
	MAX2769_TEST2           = 0b1001        // 0x14C0402
} max2769_regaddr;

/*! \typedef adis_config
 *
 * Configuration for the ADIS connections
 */
typedef struct
{
	struct pin      spi_sck;                /*! \brief The SPI SCK wire */
	struct pin      spi_mosi;               /*! \brief The SPI MOSI wire */
	struct pin      spi_cs;                 /*! \brief The SPI CS wire */
	SPIDriver   *   SPID;                   /*! \brief the SPI driver */
	struct pin      idle;                   /*! \brief low power idle */
	struct pin      shdn;                   /*! \brief Shutdown device */
	struct pin      q1_timesync;
	struct pin      q0_datasync;
	struct pin      ld;
	struct pin      antflag;
	struct pin      i1_clk_ser;
	struct pin      i0_data_out;
} MAX2769Config;

extern const MAX2769Config            max2769_gps;

void max2769_get_data(ADIS16405Data * data);
uint16_t max2769_get(max2769_regaddr addr);

void max2769_set(max2769_regaddr addr, uint16_t value);
void max2769_reset(void);

void max2769_init(const MAX2769Config * conf);

#ifdef __cplusplus
}
#endif

#endif
