/*! \file MAX2769.c
 *
 * API to support transactions through the SPI port to
 * the Maxim Integrated MAX2769 Universal GPS Receiver
 * Website:  www.maximintegrated.com
 *
 */

#include "ch.h"
#include "hal.h"

#include "utils_hal.h"
#include "usbdetail.h"
#include "chprintf.h"

#include "MAX2769.h"

#define                 UNUSED                  __attribute__((unused))

#define                 WRITE_ADDR(addr)        ((addr) |  0x80)
#define                 READ_ADDR(addr)         ((addr) & ~0x80)
#define                 MAX2769_DATA_LENGTH     254

uint8_t                 max2769_raw_in[MAX2769_DATA_LENGTH];

static const MAX2769Config * CONF;

EventSource MAX2769_write_done;

const MAX2769Config max2769_gps =
{
	.spi_sck     = {GPIOE, GPIOB_MAX_CFG_SCLK},
	.spi_mosi    = {GPIOE, GPIOB_MAX_CFG_MOSI},
	.spi_cs      = {GPIOE, GPIOE_MAX_CFG_CS},
	.SPID        = &SPID1,
	.idle        = {GPIOE, GPIOE_MAX_IDLE},
	.shdn        = {GPIOE, GPIOE_MAX_SHDN},
	.q1_timesync = {GPIOE, GPIOE_TIMESYNC},
	.q0_datasync = {GPIOE, GPIOE_DATASYNC},
	.ld          = {GPIOB, GPIOB_LD},
	.antflag     = {GPIOB, GPIOB_ANTFLAG},
	.i1_clk_ser  = {GPIOA, GPIOA_CLK_SER},
	.i0_data_out = {GPIOB, GPIOB_DATA_OUT}
};

/* MAX2769 is write only device */
void max2769_set(max2769_regaddr addr, uint32_t value)
{
	uint8_t txbuf[4] =
	{
//		     xxxx xxxx            xxxx xxxx                     xxxx xxxx              xxxx          aaaa
		value&0xff00000>>20,       value& 0xff000 >> 12,      (val & 0xff0)>>4,     ((val & 0xf) << 4)| addr
	};
	CONF->SPID->spi->CR1 |= SPI_CR1_BIDIOE;
	spiAcquireBus(CONF->SPID);
	palClearPad(CONF->spi_cs.port, CONF->spi_cs.pad);
	spiSelect(CONF->SPID);
	spiSend(CONF->SPID, sizeof(txbuf), txbuf);
	spiUnselect(CONF->SPID);
	palSetPad(CONF->spi_cs.port, CONF->spi_cs.pad);
	spiReleaseBus(CONF->SPID);
}


/*! \brief Reset the MAX2769
 *
 * Theory: No reset on MAX2769, but there is a shutdown pin
 *         Use it instead.
 */
void max2769_reset()
{
	const unsigned int MAX2769_RESET_MSECS = 500;
	palClearPad(CONF->shdn.port, CONF->shdn.pad);
	chThdSleepMilliseconds(MAX2769_RESET_MSECS);
	palSetPad(CONF->shdn.port, CONF->shdn.pad);
}

static void spi_complete(SPIDriver * SPID){
    chSysLockFromIsr();
    spiUnselectI(SPID);
    chEvtBroadcastI(&MAX2769_write_done);
    chSysUnlockFromIsr();
//    spiReleaseBus(CONF->SPID); TODO
}

void  max2769_config()
{
	int i = 0;
	// max2769_set(MAX2769_CONF1, 0b0000 1011 1100 1111 0101 0001 1010 0011 )
	//max2769_set(MAX2769_CONF1, 0x0BCF51A3 )
	// Turn on LNA1
	while(1)
	{
		max2769_set(MAX2769_CONF1, (MAX2769_CONF1_DEF & (1 << 14)) );
		// Turn on LNA2
		max2769_set(MAX2769_CONF1, (MAX2769_CONF1_DEF & (1 << 13)) );
		chThdSleepMilliseconds(3000);
		// Turn on LNA1
		max2769_set(MAX2769_CONF1, (MAX2769_CONF1_DEF & ~(1 << 14)) );
		// Turn on LNA2
		max2769_set(MAX2769_CONF1, (MAX2769_CONF1_DEF & ~(1 << 13)) );
		chThdSleepMilliseconds(3000);
	}
}



void max2769_init(const MAX2769Config * conf)
{
	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST |
	                   PAL_STM32_PUDR_FLOATING;
	/* SPI pins setup */
	palSetPadMode(conf->spi_sck.port, conf->spi_sck.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_mosi.port, conf->spi_mosi.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_cs.port, conf->spi_cs.pad, PAL_MODE_OUTPUT_PUSHPULL | PINMODE);
	palSetPad(conf->spi_cs.port, conf->spi_cs.pad); //unselect
	/* GPIO pins setup */
	/* MAX2869 SPI configuration    TBD as of Wed 11 June 2014 21:02:06 (PDT)
	 * 656250Hz, CPHA=1, CPOL=1, MSb first.
	 */
	static SPIConfig spicfg =
	{
		.end_cb = spi_complete,
		.cr1    = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE
	};
	spicfg.ssport = conf->spi_cs.port;
	spicfg.sspad  = conf->spi_cs.pad;
	spiStart(conf->SPID, &spicfg);
	CONF = conf;
}



