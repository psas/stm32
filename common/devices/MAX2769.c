/*! \file MAX2769.c
 *
 * API to support transactions through the SPI port to
 * the Maxim Integrated MAX2769 Universal GPS Receiver
 * Website:  www.maximintegrated.com
 *
 */

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_hal.h"
#include "usbdetail.h"
#include "chprintf.h"

#include "MAX2769.h"

static const MAX2769Config * CONF;

EVENTSOURCE_DECL(MAX2769_write_done);

const MAX2769Config max2769_gps =
{
	.spi_sck     = {GPIOB, GPIOB_MAX_CFG_SCLK},
	.spi_mosi    = {GPIOB, GPIOB_MAX_CFG_MOSI},
	.spi_miso    = {GPIOB, GPIOB_MAX_CFG_MISO},
	.spi_cs      = {GPIOE, GPIOE_MAX_CFG_CS},
	.SPIDCONFIG  = &SPID2,
	.SPIDREAD    = &SPID1,
	.idle        = {GPIOE, GPIOE_MAX_IDLE},
	.shdn        = {GPIOE, GPIOE_MAX_SHDN},
	.q1_timesync = {GPIOE, GPIOE_TIMESYNC},
	.q0_datasync = {GPIOE, GPIOE_DATASYNC},
	.ld          = {GPIOB, GPIOB_LD},
	.antflag     = {GPIOB, GPIOB_ANTFLAG},
	.i1_clk_ser  = {GPIOA, GPIOA_CLK_SER},
	.i0_data_out = {GPIOB, GPIOB_DATA_OUT},
	.spi1_nss    = {GPIOA, GPIOA_PIN4}
};

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

/* MAX2769 is write only device (!?) */
void max2769_set(max2769_regaddr addr, uint32_t value)
{
	// Address is last 4 bits of data write ... data writes first.
	value  = value << 4;
	value |= (addr & 0xf);
	uint8_t txbuf[4] =
	{
		(value & 0xff000000) >> 24, (value & 0xff0000) >> 16, (value & 0xff00) >> 8, (value & 0xff)
	};
	spiAcquireBus(CONF->SPIDCONFIG);
	spiSelect(CONF->SPIDCONFIG);
	spiSend(CONF->SPIDCONFIG, sizeof(txbuf), txbuf);
	spiUnselect(CONF->SPIDCONFIG);
	spiReleaseBus(CONF->SPIDCONFIG);
}

#define GPS_BUF_SIZE 700
static int16_t gps_buf1[GPS_BUF_SIZE];
static int16_t gps_buf2[GPS_BUF_SIZE];
static int16_t * frontbuf;
static int16_t * backbuf;
void max2769_read(void){
	CONF->SPIDREAD->spi->CR1 |= SPI_CR1_SSI;
	spiStartReceive(CONF->SPIDREAD, GPS_BUF_SIZE, gps_buf1);
}

void spireadcb(SPIDriver *spip UNUSED){
	if(frontbuf == gps_buf1){
		frontbuf = gps_buf2;
		backbuf = gps_buf1;
	} else {
		frontbuf = gps_buf1;
		backbuf = gps_buf2;
	}
	chSysLockFromIsr();
	spiStartReceiveI(CONF->SPIDREAD, GPS_BUF_SIZE, frontbuf);
	chEvtBroadcastI(&MAX2769_write_done);
	chSysUnlockFromIsr();
}

void max2769_init(const MAX2769Config * conf)
{
	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST |
	                   PAL_STM32_PUDR_FLOATING;
	/* SPI pins setup */
	palSetPadMode(conf->spi_sck.port, conf->spi_sck.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_mosi.port, conf->spi_mosi.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_miso.port, conf->spi_miso.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_cs.port, conf->spi_cs.pad, PAL_MODE_OUTPUT_PUSHPULL | PINMODE);
	palSetPad(conf->spi_cs.port, conf->spi_cs.pad); //unselect
	palSetPad(conf->spi1_nss.port, conf->spi1_nss.pad); //unselect
	/* GPIO pins setup */
	/*
	 * MAX2869 SPI configuration
	 */
	static SPIConfig spicfg =
	{
		.end_cb = NULL,
		.cr1 = SPI_CR1_BR_2 | SPI_CR1_BR_1
	};
	spicfg.ssport = conf->spi_cs.port;
	spicfg.sspad  = conf->spi_cs.pad;
	spiStart(conf->SPIDCONFIG, &spicfg);


	static SPIConfig spiread =
	{
		.end_cb = spireadcb,
		.cr1 = SPI_CR1_DFF
	};
	spiStart(conf->SPIDREAD, &spiread);
	conf->SPIDREAD->spi->CR1 &= ~SPI_CR1_SPE;
	conf->SPIDREAD->spi->CR1 &= ~SPI_CR1_MSTR;
	conf->SPIDREAD->spi->CR1 |= SPI_CR1_SSM | SPI_CR1_RXONLY | SPI_CR1_CPHA;
	conf->SPIDREAD->spi->CR1 &= SPI_CR1_SPE;

	CONF = conf;
}



