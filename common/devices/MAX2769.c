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

EVENTSOURCE_DECL(MAX2769_read_done);

/*! \brief Reset the MAX2769
 *
 * Theory: No reset on MAX2769, but there is a shutdown pin
 *         Use it instead.
 */
void max2769_reset() {
	const unsigned int MAX2769_RESET_MSECS = 500;
	palClearPad(CONF->max.shdn.port, CONF->max.shdn.pad);
	chThdSleepMilliseconds(MAX2769_RESET_MSECS);
	palSetPad(CONF->max.shdn.port, CONF->max.shdn.pad);
}
/* MAX2769 is write only device (!?) */
void max2769_set(max2769_regaddr addr, uint32_t value) {
	// Address is last 4 bits of data write ... data writes first.
	value  = value << 4;
	value |= (addr & 0xf);
	uint8_t txbuf[4] = {
		(value & 0xff000000) >> 24,
		(value & 0xff0000) >> 16,
		(value & 0xff00) >> 8,
		(value & 0xff)
	};
	spiAcquireBus(CONF->max.SPID);
	spiSelect(CONF->max.SPID);
	spiSend(CONF->max.SPID, sizeof(txbuf), txbuf);
	spiUnselect(CONF->max.SPID);
	spiReleaseBus(CONF->max.SPID);
}

static bool frontbuf;
static bool inuse;
static void max2769_read(void) {
	spiStartReceive(CONF->cpld.SPID, GPS_BUFFER_SIZE, CONF->bufs[frontbuf]);
}

static void spicpldcb(SPIDriver *spip UNUSED) {
	if(!inuse) {
		frontbuf = !frontbuf;
	}

	while(!palReadPad(CONF->cpld.nss.port, CONF->cpld.nss.pad));
	spiStartReceiveI(CONF->cpld.SPID, GPS_BUFFER_SIZE, CONF->bufs[frontbuf]);

	chSysLockFromIsr();
	chEvtBroadcastI(&MAX2769_read_done);
	chSysUnlockFromIsr();
}

uint8_t * max2769_getdata(void) {
	inuse = true;
	return CONF->bufs[!frontbuf];
}

void max2769_donewithdata(void) {
	inuse = false;
}

void max2769_init(const MAX2769Config * conf) {

	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_PUDR_FLOATING;
	uint32_t NSSMODE = PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST;
	/* config SPI setup */
	palSetPadMode(conf->max.mosi.port, conf->max.mosi.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->max.sck.port, conf->max.sck.pad,   PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->max.nss.port, conf->max.nss.pad, NSSMODE | PINMODE);
	palSetPad(conf->max.nss.port, conf->max.nss.pad); //unselect

	static SPIConfig spimax = {
		.end_cb = NULL,
		.cr1 =  SPI_CR1_BR_1
	};
	spimax.ssport = conf->max.nss.port;
	spimax.sspad  = conf->max.nss.pad;
	spiStart(conf->max.SPID, &spimax);

	/* Input SPI setup */
	palSetPadMode(conf->cpld.mosi.port, conf->cpld.mosi.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->cpld.sck.port, conf->cpld.sck.pad,   PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->cpld.nss.port, conf->cpld.nss.pad,   PAL_MODE_ALTERNATE(5));

	static SPIConfig spicpld = {
		.end_cb = spicpldcb,
		.cr1 = 0
	};
	spiStart(conf->cpld.SPID, &spicpld);
	conf->cpld.SPID->spi->CR1 &= ~SPI_CR1_SPE; //Disable peripheral
	conf->cpld.SPID->spi->CR1 &= ~SPI_CR1_MSTR & ~SPI_CR1_SSI & ~SPI_CR1_SSM; //clear
	conf->cpld.SPID->spi->CR2 &= ~SPI_CR2_SSOE;
	conf->cpld.SPID->spi->CR1 |= SPI_CR1_RXONLY;// | SPI_CR1_CPHA; //set
	conf->cpld.SPID->spi->CR1 |= SPI_CR1_SPE; //Re-enable peripheral
	CONF = conf;

	palClearPad(conf->cpld.debug.port, conf->cpld.debug.pad);

	palClearPad(conf->cpld.reset.port, conf->cpld.reset.pad);
	chThdSleepMilliseconds(1);
	pwmStart(conf->cpld.PWMD, conf->cpld.clk_src_cfg);
	pwmEnableChannel(conf->cpld.PWMD, 0, 1);
	chThdSleepMilliseconds(1);
	palSetPad(conf->cpld.reset.port, conf->cpld.reset.pad);

	while(!palReadPad(conf->cpld.nss.port, conf->cpld.nss.pad));
	max2769_read();
}



