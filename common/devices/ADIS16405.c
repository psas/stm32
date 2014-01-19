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

#include "usbdetail.h"
#include "chprintf.h"

#include "ADIS16405.h"

#define WRITE_ADDR(addr) (addr) |  0x80
#define READ_ADDR(addr)  (addr) & ~0x80

static SPIDriver *SPID = NULL;
static const adis_pins * PINS = NULL;
EventSource adis_data_ready;
uint8_t adis_raw_in[sizeof(ADIS16405_burst_data)+ 2];

const char adisid[(sizeof("ADIS")-1)] = "ADIS";

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


static int16_t sign_extend(uint16_t val, int bits) {
    if((val&(1<<(bits-1))) != 0){
        val = val - (1<<bits);
    }
    return val;
}

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
//    todo: timestamp here to measure delay in event broadcasts
    chEvtBroadcastI(&adis_data_ready);
    chSysUnlockFromIsr();
//    spiReleaseBus(SPID); TODO
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

void adis_get_data(ADIS16405_burst_data * data){ // TODO: adis error struct
    // provides last sample or 0s if no sample received.
    chSysLock();
    buffer_to_burst_data(adis_raw_in + 2, data);
    chSysUnlock();
}


//void adis_auto_diagnostic(){
//    uint16_t msc = adis_get(ADIS_MSC_CTRL);
//    adis_set(ADIS_MSC_CTRL, msc | 1<<10);
//}

/*! \brief Reset the ADIS
 */
void adis_reset() {
    const unsigned int ADIS_RESET_MSECS = 500;
	palClearPad(PINS->reset.port, PINS->reset.pad);
	chThdSleepMilliseconds(ADIS_RESET_MSECS);
	palSetPad(PINS->reset.port, PINS->reset.pad);
}
