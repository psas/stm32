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

uint8_t                 max2769_raw_in[MAX2769_DATA_LENGTH];

static const MAX2769Config * CONF;

const MAX2769Config max2769_gps =
{
	.spi_sck     = {GPIOE, GPIOE_PIN13},
	.spi_mosi    = {GPIOE, GPIOE_PIN14},
	.spi_cs      = {GPIOE, GPIOE_PIN12},
	.SPID        = &SPID1,
	.idle        = {GPIOE, GPIOE_PIN11},
	.shdn        = {GPIOE, GPIOE_PIN10},
	.q1_timesync = {GPIOE, GPIOE_PIN9},
	.q0_datasync = {GPIOE, GPIOE_PIN7},
	.ld          = {GPIOB, GPIOB_PIN2},
	.antflag     = {GPIOB, GPIOB_PIN1},
	.i1_clk_ser  = {GPIOA, GPIOE_PIN5},
	.i0_data_out = {GPIOB, GPIOE_PIN5}
};

uint16_t max2769_get(max2769_regaddr addr)  //todo: array
{
	uint8_t txbuf[2] = {addr, 0};
	uint8_t rxbuf[2];
	spiAcquireBus(CONF->SPID);
	spiSelect(CONF->SPID);
	spiSend(CONF->SPID, sizeof(txbuf), txbuf);
	spiReceive(CONF->SPID, sizeof(rxbuf), rxbuf);
	spiUnselect(CONF->SPID);
	spiReleaseBus(CONF->SPID);
	return rxbuf[0] << 8 | rxbuf[1];
}

void max2769_set(max2769_regaddr addr, uint16_t value)  //todo:array
{
	uint8_t txbuf[4] =
	{
		WRITE_ADDR(addr + 1), value >> 8,
		WRITE_ADDR(addr),   value
	};
	spiAcquireBus(CONF->SPID);
	spiSelect(CONF->SPID);
	spiSend(CONF->SPID, sizeof(txbuf), txbuf);
	spiUnselect(CONF->SPID);
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

void max2769_init(const MAX2769Config * conf)
{
	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST |
	                   PAL_STM32_PUDR_FLOATING;
	/* SPI pins setup */
	palSetPadMode(conf->spi_sck.port, conf->spi_sck.pad, PAL_MODE_ALTERNATE(X) | PINMODE);
	palSetPadMode(conf->spi_miso.port, conf->spi_miso.pad, PAL_MODE_ALTERNATE(X) | PINMODE);
	palSetPadMode(conf->spi_mosi.port, conf->spi_mosi.pad, PAL_MODE_ALTERNATE(X) | PINMODE);
	palSetPadMode(conf->spi_cs.port, conf->spi_cs.pad, PAL_MODE_OUTPUT_PUSHPULL | PINMODE);
	palSetPad(conf->spi_cs.port, conf->spi_cs.pad); //unselect
	/* GPIO pins setup */
	palSetPadMode(conf->reset.port, conf->reset.pad,
	              PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio1.port, conf->dio1.pad,
	              PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio2.port, conf->dio2.pad,
	              PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio3.port, conf->dio3.pad,
	              PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio4.port, conf->dio4.pad,
	              PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	/* MAX2869 SPI configuration    TBD as of Wed 11 June 2014 21:02:06 (PDT)
	 * 656250Hz, CPHA=1, CPOL=1, MSb first.
	 */
	static SPIConfig spicfg =
	{
		.end_cb = spi_complete,
		.cr1 = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
	};
	spicfg.ssport = conf->spi_cs.port;
	spicfg.sspad = conf->spi_cs.pad;
	spiStart(conf->SPID, &spicfg);
	CONF = conf;
}


