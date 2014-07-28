/*! \file ADIS16405.c
 *
 * API to support transactions through the SPI port to
 * the Analog Devices ADIS16405 series IMU.
 *
 */

#include "ch.h"
#include "hal.h"

#include "utils_hal.h"
#include "usbdetail.h"
#include "chprintf.h"

#include "ADIS16405.h"

#define UNUSED __attribute__((unused))

#define WRITE_ADDR(addr) ((addr) |	0x80)
#define READ_ADDR(addr)  ((addr) & ~0x80)

#define BURST_EXCHANGE_LEN (sizeof(ADIS16405Data)+ 2) //+2 for initial addr
uint8_t adis_raw_in[BURST_EXCHANGE_LEN];
EventSource ADIS16405_data_ready;

static const ADIS16405Config * CONF;

const ADIS16405Config adis_olimex_e407 = {
	.spi_cs = {GPIOA, GPIOA_PIN4},
	.spi_sck = {GPIOA, GPIOA_PIN5},
	.spi_miso = {GPIOA, GPIOA_PIN6},
	.spi_mosi = {GPIOB, GPIOB_PIN5},
	.SPID = &SPID1,
	.reset = {GPIOD, GPIOD_PIN8},
	.dio1 = {GPIOD, GPIOD_PIN9},
	.dio2 = {GPIOD, GPIOD_PIN10},
	.dio3 = {GPIOD, GPIOD_PIN11},
	.dio4 = {GPIOD, GPIOD_PIN12},
};

static void adis_data_ready(EXTDriver *extp UNUSED, expchannel_t channel UNUSED)
{
	static uint8_t address[BURST_EXCHANGE_LEN] = {0x3E, 0};
//	spiAcquireBus(CONF->SPID);TODO
	chSysLockFromIsr();
	spiSelectI(CONF->SPID);
	spiStartExchangeI(CONF->SPID, BURST_EXCHANGE_LEN, address, adis_raw_in);
	chSysUnlockFromIsr();
}

static void spi_complete(SPIDriver * SPID){
	chSysLockFromIsr();
	spiUnselectI(SPID);
//	todo: timestamp here to measure delay in event broadcasts
	chEvtBroadcastI(&ADIS16405_data_ready);
	chSysUnlockFromIsr();
//	spiReleaseBus(CONF->SPID); TODO
}

void adis_init(const ADIS16405Config * conf) {
	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING;
	/* SPI pins setup */
	palSetPadMode(conf->spi_sck.port, conf->spi_sck.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_miso.port, conf->spi_miso.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_mosi.port, conf->spi_mosi.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(conf->spi_cs.port, conf->spi_cs.pad, PAL_MODE_OUTPUT_PUSHPULL | PINMODE);
	palSetPad(conf->spi_cs.port, conf->spi_cs.pad); //unselect
	/* GPIO pins setup */
	palSetPadMode(conf->reset.port, conf->reset.pad, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio1.port, conf->dio1.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio2.port, conf->dio2.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio3.port, conf->dio3.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(conf->dio4.port, conf->dio4.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);

	/* ADIS SPI configuration
	 * 656250Hz, CPHA=1, CPOL=1, MSb first.
	 * For burst mode ADIS SPI is limited to 1Mhz.
	 */
	static SPIConfig spicfg = {
		.end_cb = spi_complete,
		.cr1 = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
	};
	spicfg.ssport = conf->spi_cs.port;
	spicfg.sspad = conf->spi_cs.pad;
	spiStart(conf->SPID, &spicfg);

	/* Enable the external interrupt */
	chEvtInit(&ADIS16405_data_ready);
	extAddCallback(&(conf->dio1), EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART, adis_data_ready);
	extUtilsStart();

	CONF = conf;
}

uint16_t adis_get(adis_regaddr addr){ //todo: array
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

void adis_set(adis_regaddr addr, uint16_t value){ //todo:array
	uint8_t txbuf[4] = {
		WRITE_ADDR(addr+1), value >> 8,
		WRITE_ADDR(addr),   value
	};
	spiAcquireBus(CONF->SPID);
	spiSelect(CONF->SPID);

	spiSend(CONF->SPID, sizeof(txbuf), txbuf);

	spiUnselect(CONF->SPID);
	spiReleaseBus(CONF->SPID);
}


static int16_t sign_extend(uint16_t val, int bits) {
	if((val&(1<<(bits-1))) != 0){
		val = val - (1<<bits);
	}
	return val;
}

static void buffer_to_burst_data(uint8_t * raw, ADIS16405Data * data){
	//todo: check nd and ea bits

	data->supply_out = (raw[0] << 8 | raw[1]) & 0x3fff;
	data->xgyro_out  = sign_extend((raw[2]	<< 8 | raw[3]) & 0x3fff, 14);
	data->ygyro_out  = sign_extend((raw[4]	<< 8 | raw[5]) & 0x3fff, 14);
	data->zgyro_out  = sign_extend((raw[6]	<< 8 | raw[7]) & 0x3fff, 14);
	data->xaccl_out  = sign_extend((raw[8]	<< 8 | raw[9]) & 0x3fff, 14);
	data->yaccl_out  = sign_extend((raw[10] << 8 | raw[11]) & 0x3fff, 14);
	data->zaccl_out  = sign_extend((raw[12] << 8 | raw[13]) & 0x3fff, 14);
	data->xmagn_out  = sign_extend((raw[14] << 8 | raw[15]) & 0x3fff, 14);
	data->ymagn_out  = sign_extend((raw[16] << 8 | raw[17]) & 0x3fff, 14);
	data->zmagn_out  = sign_extend((raw[18] << 8 | raw[19]) & 0x3fff, 14);
	data->temp_out   = sign_extend((raw[20] << 8 | raw[21]) & 0x0fff, 12);
	data->aux_adc    = (raw[22] << 8 | raw[23]) & 0x0fff;
}

void adis_get_data(ADIS16405Data * data){ // TODO: adis error struct
	// provides last sample or 0s if no sample received.
	chSysLock();
	buffer_to_burst_data(adis_raw_in + 2, data); //first 2 bytes are padding
	chSysUnlock();
}


uint16_t adis_self_test(void){

	extChannelDisable(&EXTD1,CONF->dio1.pad);

	// DIAG_STAT clears after each read so we read to clear it
	uint16_t diagstat = adis_get(ADIS_DIAG_STAT);

	uint16_t msc = adis_get(ADIS_MSC_CTRL);
	adis_set(ADIS_MSC_CTRL, msc | 1<<10);

	do {
		msc = adis_get(ADIS_MSC_CTRL);
	} while (msc & 1<< 10);

	diagstat = adis_get(ADIS_DIAG_STAT);

	extChannelEnable(&EXTD1,CONF->dio1.pad);

	return diagstat;
}

/*! \brief Reset the ADIS
 */
void adis_reset(void) {
	const unsigned int ADIS_RESET_MSECS = 500;
	palClearPad(CONF->reset.port, CONF->reset.pad);
	chThdSleepMilliseconds(ADIS_RESET_MSECS);
	palSetPad(CONF->reset.port, CONF->reset.pad);
}
