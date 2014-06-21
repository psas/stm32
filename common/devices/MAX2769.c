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
	.spi_sck     = {GPIOB, GPIOB_MAX_CFG_SCLK},
	.spi_mosi    = {GPIOB, GPIOB_MAX_CFG_MOSI},
	.spi_miso    = {GPIOB, GPIOB_MAX_CFG_MISO},
	.spi_cs      = {GPIOE, GPIOE_MAX_CFG_CS},
	.SPID        = &SPID2,
	.idle        = {GPIOE, GPIOE_MAX_IDLE},
	.shdn        = {GPIOE, GPIOE_MAX_SHDN},
	.q1_timesync = {GPIOE, GPIOE_TIMESYNC},
	.q0_datasync = {GPIOE, GPIOE_DATASYNC},
	.ld          = {GPIOB, GPIOB_LD},
	.antflag     = {GPIOB, GPIOB_ANTFLAG},
	.i1_clk_ser  = {GPIOA, GPIOA_CLK_SER},
	.i0_data_out = {GPIOB, GPIOB_DATA_OUT}
};

/* MAX2769 is write only device (!?) */
void max2769_set(max2769_regaddr addr, uint32_t value)
{
	//BaseSequentialStream *chp;
    //chp = getUsbStream();
		//Xxxxxxxx
	value  = value << 4;
		//X xxxxxxx0
    value |= (addr & 0xf);
	uint8_t txbuf[4] =
	{
			(value & 0xff000000) >> 24, (value & 0xff0000) >> 16, (value & 0xff00) >> 8, (value & 0xff)
	};
	//CONF->SPID->spi->CR1 |= SPI_CR1_BIDIOE;   This is set in init...
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

static void spi_complete(SPIDriver * SPID)
{
	chSysLockFromIsr();
	spiUnselectI(SPID);
	chEvtBroadcastI(&MAX2769_write_done);
	chSysUnlockFromIsr();
	//    spiReleaseBus(CONF->SPID); TODO
}

void  max2769_test_lna()
{
	BaseSequentialStream *chp;
    chp = getUsbStream();

	int new_conf1 = MAX2769_CONF1_DEF;

	chThdSleepMilliseconds(4000);

	// Turn on LNA1
	while(1)
	{
		// Turn off ANTEN
		new_conf1 &= ~(0b1 << MAX2769_CONF1_ANTEN);
		chprintf(chp, "ANTEN OFF: 0x%x\r\n", new_conf1);
		max2769_set(MAX2769_CONF1, new_conf1 );
		chThdSleepMilliseconds(1000);

		// Turn on ANTEN
		//new_conf1 |= (0b1 << MAX2769_CONF1_ANTEN);
		//chprintf(chp, "ANTEN ON: 0x%x\r\n", new_conf1);
		//max2769_set(MAX2769_CONF1, new_conf1 );
		//chThdSleepMilliseconds(1000);
	}
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
	/* GPIO pins setup */
	/* MAX2869 SPI configuration 
	 * 656250Hz, CPHA=1, CPOL=1, MSb first.
	 */
	static SPIConfig spicfg =
	{
		.end_cb = NULL,
		//.cr1    = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE
		.cr1    = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1 
	};
	spicfg.ssport = conf->spi_cs.port;
	spicfg.sspad  = conf->spi_cs.pad;
	spiStart(conf->SPID, &spicfg);
	CONF = conf;
}

