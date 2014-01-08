/*! \file ADIS16405.c
 *
 * API to support transactions through the SPI port to
 * the Analog Devices ADIS16405 series IMU.
 *
 */

/*! \defgroup adis16405 ADIS IMU
 *
 * @{
 */

#include "ch.h"
#include "hal.h"

#if ADIS_DEBUG || defined(__DOXYGEN__)
#include "usbdetail.h"
#include "chprintf.h"
#endif

#include "usbdetail.h"
#include "chprintf.h"

#include "ADIS16405.h"

#define WRITE_ADDR(addr) (addr) |  0x80
#define READ_ADDR(addr)  (addr) & ~0x80

static SPIDriver *SPID = NULL;
static const adis_pins * PINS = NULL;
EventSource adis_data_ready;
ADIS16405_burst_data adis16405_burst_data;
uint8_t adis_raw_in[sizeof(ADIS16405_burst_data)+ 2];


const adis_pins adis_olimex_e407 = {
        .spi_cs = {GPIOA, GPIOA_PIN4},
        .spi_sck = {GPIOA, GPIOA_PIN5},
        .spi_miso = {GPIOA, GPIOA_PIN6},
        .spi_mosi = {GPIOB, GPIOB_PIN5},
        .reset = {GPIOD, GPIOD_PIN8},
        .dio1 = {GPIOD, GPIOD_PIN9},
        .dio2 = {GPIOD, GPIOD_PIN10},
        .dio3 = {GPIOD, GPIOD_PIN11},
        .dio4 = {GPIOD, GPIOD_PIN12},
};


static void buffer_to_burst_data(uint8_t * raw, ADIS16405_burst_data * data){
    //todo: check nd and ea bits

    data->supply_out = (raw[0] << 8 | raw[1]) & 0x3fff;
    data->xgyro_out  = sign_extend((raw[2]  << 8 | raw[3]) & 0x3fff, 14);
    data->ygyro_out  = sign_extend((raw[4]  << 8 | raw[5]) & 0x3fff, 14);
    data->zgyro_out  = sign_extend((raw[6]  << 8 | raw[7]) & 0x3fff, 14);
    data->xaccl_out  = sign_extend((raw[8]  << 8 | raw[9]) & 0x3fff, 14);
    data->yaccl_out  = sign_extend((raw[10] << 8 | raw[11]) & 0x3fff, 14);
    data->zaccl_out  = sign_extend((raw[12] << 8 | raw[13]) & 0x3fff, 14);
    data->xmagn_out  = sign_extend((raw[14] << 8 | raw[15]) & 0x3fff, 14);
    data->ymagn_out  = sign_extend((raw[16] << 8 | raw[17]) & 0x3fff, 14);
    data->zmagn_out  = sign_extend((raw[18] << 8 | raw[19]) & 0x3fff, 14);
    data->temp_out   = sign_extend((raw[20] << 8 | raw[21]) & 0x0fff, 12);
    data->aux_adc    = (raw[22] << 8 | raw[23]) & 0x0fff;
}



static void adis_data_rdy_cb(EXTDriver *extp __attribute__((unused)),
                             expchannel_t channel __attribute__((unused)))
{
    static uint8_t address[sizeof(ADIS16405_burst_data)+2] = {0x3E, 0};
//    spiAcquireBus(SPID);TODO
    chSysLockFromIsr();
    spiSelectI(SPID);
    spiStartExchangeI(SPID, sizeof(ADIS16405_burst_data)+2, address, adis_raw_in);
    chSysUnlockFromIsr();
}

static void spi_complete_cb(SPIDriver * SPID){
    chSysLockFromIsr();
    spiUnselectI(SPID);
    chEvtBroadcastI(&adis_data_ready);
    chSysUnlockFromIsr();
//    spiReleaseBus(SPID); TODO
}


void get_adis_data(ADIS16405_burst_data * data){ //adis error struct
    // provides last sample or 0s if no sample received.
    chSysLock();
    buffer_to_burst_data(adis_raw_in + 2, adis16405_burst_data);
    chSysUnlock();
}

void adis_init(const adis_pins * pins) {

    PINS = pins;
    uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING;
    /* SPI pins setup */
    palSetPadMode(pins->spi_sck.port, pins->spi_sck.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
    palSetPadMode(pins->spi_miso.port, pins->spi_miso.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
    palSetPadMode(pins->spi_mosi.port, pins->spi_mosi.pad, PAL_MODE_ALTERNATE(5) | PINMODE);
    palSetPadMode(pins->spi_cs.port, pins->spi_cs.pad, PAL_MODE_OUTPUT_PUSHPULL | PINMODE);
    palSetPad(pins->spi_cs.port, pins->spi_cs.pad); //unselect
    /* GPIO pins setup */
    palSetPadMode(pins->reset.port, pins->reset.pad, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(pins->dio1.port, pins->dio1.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(pins->dio2.port, pins->dio2.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(pins->dio3.port, pins->dio3.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(pins->dio4.port, pins->dio4.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);


    /*! \brief ADIS SPI configuration
     *
     * 656250Hz, CPHA=1, CPOL=1, MSb first.
     *
     * For burst mode ADIS SPI is limited to 1Mhz.
     */

    static SPIConfig spicfg = {
            .end_cb = spi_complete_cb,
            .cr1 = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
    };
    spicfg.ssport = pins->spi_cs.port;
    spicfg.sspad = pins->spi_cs.pad;
    SPID = &SPID1;
    spiStart(SPID, &spicfg);


    /* Enable the external interrupt */
    chEvtInit(&adis_data_ready);

    //TODO: Is there a better way than a deep copy of doing this so that it
    //      doesn't leave a bunch of unused extconfigs laying around?
    //      EXTDn.config is marked const...
    static EXTConfig extconfig;
    if(EXTD1.config != NULL){
        int i;
        for(i = 0; i < EXT_MAX_CHANNELS; ++i){
            extconfig.channels[i].cb = EXTD1.config->channels[i].cb;
            extconfig.channels[i].mode = EXTD1.config->channels[i].mode;
        }
    }

    //set dio1
    uint32_t CHANNEL_MODE = EXT_MODE_GPIOD;
//    TODO:
//    if(pins->dio1.port == GPIOA){
//        CHANNEL_MODE = EXT_MODE_GPIOA;
//    } else if(pins->dio1.port == GPIOB){

    //TODO: a good way to not blindly overwrite the already existing channel cb/mode
    extconfig.channels[pins->dio1.pad].cb = adis_data_rdy_cb;
    extconfig.channels[pins->dio1.pad].mode = EXT_CH_MODE_RISING_EDGE | CHANNEL_MODE | EXT_CH_MODE_AUTOSTART;

    extStart(&EXTD1, &extconfig);

    adis_reset();
}

//#if ADIS_DEBUG || defined(__DOXYGEN__)
//	/*! \brief Convert an ADIS 14 bit accel. value to micro-g
//	 *
//	 * @param decimal
//	 * @param accel reading
//	 * @return   TRUE if less than zero, FALSE if greater or equal to zero
//	 */
//	static bool adis_accel2ug(uint32_t* decimal, uint16_t* twos_num) {
//		uint16_t ones_comp;
//		bool     isnegative = false;
//
//		//! bit 13 is 14-bit two's complement sign bit
//		isnegative   = (((uint16_t)(1<<13) & *twos_num) != 0) ? true : false;
//
//		if(isnegative) {
//			ones_comp    = ~(*twos_num & (uint16_t)0x3fff) & 0x3fff;
//			*decimal     = (ones_comp) + 1;
//		} else {
//			*decimal     = *twos_num;
//		}
//		*decimal     *= 3330;
//		return isnegative;
//	}
//#endif


static int16_t sign_extend(uint16_t val, int bits) {
    if((val&(1<<(bits-1))) != 0){
        val = val - (1<<bits);
    }
    return val;
}

uint16_t adis_get(adis_regaddr addr){ //todo: array
    uint8_t txbuf[2] = {addr, 0};
    uint8_t rxbuf[2];

    spiAcquireBus(SPID);
    spiSelect(SPID);

    spiSend(SPID, sizeof(txbuf), txbuf);
    spiReceive(SPID, sizeof(rxbuf), rxbuf);

    spiUnselect(SPID);
    spiReleaseBus(SPID);

    return rxbuf[0] << 8 | rxbuf[1];
}

void adis_set(adis_regaddr addr, uint16_t value){ //todo:array
    uint8_t txbuf[4] = {WRITE_ADDR(addr+1), value >> 8, WRITE_ADDR(addr), value};

    spiAcquireBus(SPID);
    spiSelect(SPID);

    spiSend(SPID, sizeof(txbuf), txbuf);

    spiUnselect(SPID);
    spiReleaseBus(SPID);
}


void adis_get_burst(ADIS16405_burst_data * data){
    uint8_t address[2] = {0x3E, 0};
    uint8_t raw[sizeof(ADIS16405_burst_data)];

    spiAcquireBus(SPID);
    spiSelect(SPID);

    spiSend(SPID, sizeof(address), address);
    spiReceive(SPID, sizeof(raw), raw);

    spiUnselect(SPID);
    spiReleaseBus(SPID);

    buffer_to_burst_data(raw, data);
}

//void adis_auto_diagnostic(){
//    uint16_t msc = adis_get(ADIS_MSC_CTRL);
//    adis_set(ADIS_MSC_CTRL, msc | 1<<10);
//}

/*! \brief Reset the ADIS
 */
void adis_reset() {
	palClearPad(PINS->reset.port, PINS->reset.pad);
	chThdSleepMilliseconds(ADIS_RESET_MSECS);
	palSetPad(PINS->reset.port, PINS->reset.pad);
	chThdSleepMilliseconds(ADIS_RESET_MSECS);
}

/*!
 * t_stall is 9uS according to ADIS datasheet.
 */
void adis_tstall_delay() {
	volatile uint32_t i, j;

	/*!
	 *  This is the ADIS T_stall time.
	 *  Counting to 100 takes about 11uS.
	 *  Measured on oscilloscope.
	 *
	 *  \todo Use a HW (not virtual) timer/counter unit to generate T_stall delays
	 */
	j = 0;
	for(i=0; i<100; ++i) {
		j++;
	}
}

/*!
 *
 * @param spip
 */
//void adis_release_bus(eventid_t id) {
//	(void) id;
//	spiReleaseBus(adis_driver.spi_instance);
//}
//
///*! \brief adis_spi_cb
// *
// * What happens at end of ADIS SPI transaction
// *
// * This is executed during the SPI interrupt.
// *
// * @param spip
// */
//void adis_spi_cb(SPIDriver *spip) {
//	chSysLockFromIsr();
//
//	uint8_t       i                              = 0;
//
//	chDbgCheck(adis_driver.spi_instance == spip, "adis_spi_cb driver mismatch");
//	if(adis_driver.state == ADIS_TX_PEND) {
//		chEvtBroadcastI(&adis_spi_cb_txdone_event);
//	} else {
//		for(i=0; i<adis_driver.tx_numbytes; ++i) {
//			adis_cache_data.adis_tx_cache[i] = adis_driver.adis_txbuf[i];
//		}
//		for(i=0; i<adis_driver.rx_numbytes; ++i) {
//			adis_cache_data.adis_rx_cache[i] = adis_driver.adis_rxbuf[i];
//		}
//		adis_cache_data.reg                  = adis_driver.reg;
//		adis_cache_data.current_rx_numbytes  = adis_driver.rx_numbytes;
//		adis_cache_data.current_tx_numbytes  = adis_driver.tx_numbytes;
//		chEvtBroadcastI(&adis_spi_cb_data_captured);
//		chEvtBroadcastI(&adis_spi_cb_releasebus);
//	}
//	chSysUnlockFromIsr();
//}
//
///*! \brief Process an adis_newdata_event
// */
//void adis_newdata_handler(eventid_t id) {
//	(void)                id;
//
//	spiUnselect(adis_driver.spi_instance);                /* Slave Select de-assertion.       */
//
//	if(adis_driver.reg == ADIS_GLOB_CMD) {
//		adis16405_burst_data.adis_supply_out   = (((adis_cache_data.adis_rx_cache[0]  << 8) | adis_cache_data.adis_rx_cache[1] ) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_xgyro_out    = (((adis_cache_data.adis_rx_cache[2]  << 8) | adis_cache_data.adis_rx_cache[3] ) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_ygyro_out    = (((adis_cache_data.adis_rx_cache[4]  << 8) | adis_cache_data.adis_rx_cache[5] ) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_zgyro_out    = (((adis_cache_data.adis_rx_cache[6]  << 8) | adis_cache_data.adis_rx_cache[7] ) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_xaccl_out    = (((adis_cache_data.adis_rx_cache[8]  << 8) | adis_cache_data.adis_rx_cache[9] ) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_yaccl_out    = (((adis_cache_data.adis_rx_cache[10] << 8) | adis_cache_data.adis_rx_cache[11]) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_zaccl_out    = (((adis_cache_data.adis_rx_cache[12] << 8) | adis_cache_data.adis_rx_cache[13]) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_xmagn_out    = (((adis_cache_data.adis_rx_cache[14] << 8) | adis_cache_data.adis_rx_cache[15]) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_ymagn_out    = (((adis_cache_data.adis_rx_cache[16] << 8) | adis_cache_data.adis_rx_cache[17]) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_zmagn_out    = (((adis_cache_data.adis_rx_cache[18] << 8) | adis_cache_data.adis_rx_cache[19]) & ADIS_14_BIT_MASK );
//		adis16405_burst_data.adis_temp_out     = (((adis_cache_data.adis_rx_cache[20] << 8) | adis_cache_data.adis_rx_cache[21]) & ADIS_12_BIT_MASK );
//		adis16405_burst_data.adis_aux_adc      = (((adis_cache_data.adis_rx_cache[22] << 8) | adis_cache_data.adis_rx_cache[23]) & ADIS_12_BIT_MASK );
//
//		chEvtBroadcast(&adis_spi_burst_data_captured);         // Event monitored in data_udp_send thread
//	}
//
//#if ADIS_DEBUG
//	BaseSequentialStream    *chp = (BaseSequentialStream *)&SDU_PSAS;
//	bool                  negative = false;
//	uint32_t              result_ug = 0;
//	static uint32_t       j        = 0;
//	static uint32_t       xcount   = 0;
//
//	++j;
//	if(j>4000) {
//		//chprintf(chp, "adis driver reg: %0x%x\n", adis_driver.reg);
//		if(adis_driver.reg == ADIS_GLOB_CMD) {
//			// chprintf(chp, "%d: supply: %x %d uV\r\n", xcount, burst_data.adis_supply_out, ( burst_data.adis_supply_out * 2418));
//			//negative = twos2dec(&burst_data.adis_xaccl_out);
//			chprintf(chp, "\r\n*** ADIS ***\r\n");
//
//			chprintf(chp, "%d: T: 0x%x C\r\n", xcount, adis16405_burst_data.adis_temp_out);
//			negative   = adis_accel2ug(&result_ug, &adis16405_burst_data.adis_zaccl_out);
//			chprintf(chp, "%d: z: 0x%x  %s%d ug\r\n", xcount, adis16405_burst_data.adis_zaccl_out, (negative) ? "-" : "", result_ug);
//			negative   = adis_accel2ug(&result_ug, &adis16405_burst_data.adis_xaccl_out);
//			chprintf(chp, "%d: x: 0x%x  %s%d ug\r\n", xcount, adis16405_burst_data.adis_xaccl_out, (negative) ? "-" : "", result_ug);
//			negative   = adis_accel2ug(&result_ug, &adis16405_burst_data.adis_yaccl_out);
//			chprintf(chp, "%d: y: 0x%x  %s%d ug\r\n\r\n", xcount, adis16405_burst_data.adis_yaccl_out, (negative) ? "-" : "", result_ug);
//
//		} else if (adis_driver.reg == ADIS_PRODUCT_ID) {
//			chprintf(chp, "%d: Prod id: %x\r\n", xcount, ((adis_cache_data.adis_rx_cache[0]<< 8)|(adis_cache_data.adis_rx_cache[1])) );
//		} else if (adis_driver.reg == ADIS_TEMP_OUT) {
//			chprintf(chp, "%d: Temperature: 0x%x", xcount, (((adis_cache_data.adis_rx_cache[0] << 8) | adis_cache_data.adis_rx_cache[1]) & ADIS_12_BIT_MASK) );
//		} else {
//			chprintf(chp, "%d: not recognized %d\n", xcount, adis_driver.reg);
//		}
//
//		j=0;
//		++xcount;
//	}
//#endif
//
//	adis_driver.state             = ADIS_IDLE;   /* don't go to idle until data processed */
//}
//
//void adis_read_id_handler(eventid_t id) {
//	(void) id;
//	adis_read_id(&SPID1);
//}
//
//void adis_read_dC_handler(eventid_t id) {
//	(void) id;
//
//	adis_read_dC(&SPID1);
//}
//
//void adis_burst_read_handler(eventid_t id) {
//	(void) id;
//	//	BaseSequentialStream    *chp = (BaseSequentialStream *)&SDU_PSAS;
//	//	chprintf(chp, "+");
//
//	adis_burst_read(&SPID1);
//}
//
///*! \brief Process information from a adis_spi_cb event
// *
// */
//void adis_spi_cb_txdone_handler(eventid_t id) {
//	(void) id;
//	switch(adis_driver.reg) {
//		case ADIS_PRODUCT_ID:
//		case ADIS_TEMP_OUT:
//			spiUnselect(adis_driver.spi_instance);
//			spiReleaseBus(adis_driver.spi_instance);
//			adis_tstall_delay();
//			spiAcquireBus(adis_driver.spi_instance);
//			spiSelect(adis_driver.spi_instance);
//			spiStartReceive(adis_driver.spi_instance, adis_driver.rx_numbytes, adis_driver.adis_rxbuf);
//			adis_driver.state             = ADIS_RX_PEND;
//			break;
//		case ADIS_GLOB_CMD:
//			spiStartReceive(adis_driver.spi_instance, adis_driver.rx_numbytes, adis_driver.adis_rxbuf);
//			adis_driver.state             = ADIS_RX_PEND;
//			break;
//		default:
//			spiUnselect(adis_driver.spi_instance);
//			spiReleaseBus(adis_driver.spi_instance);
//			adis_driver.state             = ADIS_IDLE;
//			break;
//	}
//}

//! @}
