/*! \file MAX2769.h 
 */

/* Intended for use with ChibiOS RT */

#ifndef _MAX2769_H
#define _MAX2769_H

#include "spi_lld.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "ch.h"
#include "hal.h"
#include "spi_lld.h"

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

/* MAX2769 Register defaults */
typedef enum
{
	// Name                               address         default value
	MAX2769_CONF1_DEF           = 0xA2919A3,
	MAX2769_CONF2_DEF           = 0x0550288,
	MAX2769_CONF3_DEF           = 0xEAFF1DC,
	MAX2769_PLLCONF_DEF         = 0x9EC0008,
	MAX2769_DIV_DEF             = 0x0C00080,
	MAX2769_FDIV_DEF            = 0x8000070,
	MAX2769_STRM_DEF            = 0x8000000,
	MAX2769_CLK_DEF             = 0x10061B2,
	MAX2769_TEST1_DEF           = 0x1E0F401,
	MAX2769_TEST2_DEF           = 0x14C0402
} max2769_regdefault;


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

/*! \typedef
 */
typedef struct MAX2769Data {
		uint32_t some_data_tbd;
} MAX2769Data;


extern const      MAX2769Config            max2769_gps;
extern            EventSource              MAX2769_write_done;

void max2769_config(void);
void max2769_set(max2769_regaddr addr, uint16_t value);
void max2769_reset(void);

void max2769_init(const MAX2769Config * conf);

#ifdef __cplusplus
}
#endif

#endif
