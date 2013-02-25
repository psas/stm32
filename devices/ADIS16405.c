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

#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "ADIS16405.h"

EventSource        adis_newdata_event;
adis_cache         adis_cache_data;

ADIS_Driver        adis_driver;


/*! \brief Reset the ADIS
*/
void adis_reset() {
    palClearPad(adis_connections.reset_port, adis_connections.reset_pad);
    chThdSleepMilliseconds(ADIS_RESET_MSECS);
    palSetPad(adis_connections.reset_port, adis_connections.reset_pad);
    chThdSleepMilliseconds(ADIS_RESET_MSECS);
}

/*! \brief Initialize ADIS driver
 *
 */
void adis_init() {
    uint8_t     i              = 0;

    adis_driver.spi_instance   = &SPID1;
    adis_driver.state          = ADIS_IDLE;
    adis_driver.reg            = ADIS_PRODUCT_ID;

    for(i=0; i<ADIS_MAX_TX_BUFFER; ++i) {
        adis_driver.adis_txbuf[i]        = 0;
        adis_cache_data.adis_tx_cache[i] = 0;
    }
    for(i=0; i<ADIS_MAX_RX_BUFFER; ++i) {
        adis_driver.adis_rxbuf[i]        = 0xa5;
        adis_cache_data.adis_rx_cache[i] = 0xa5;
    }
}

/*! \brief adis_spi_cb
 *
 * What happens at end of ADIS SPI transaction
 *
 * This is executed during the SPI interrupt.
 *
 * @param spip
 */
void adis_spi_cb(SPIDriver *spip) {

	chSysLockFromIsr();
	uint8_t    i    = 0;

    switch(adis_driver.state) {
        case ADIS_TX_PEND:
            spiAcquireBus(spip);
            spiStart(spip, &adis_spicfg);
            spiSelect(spip);
            switch(adis_driver.reg) {
                case ADIS_PRODUCT_ID:
                    spiStartReceive(spip, 2, adis_driver.adis_rxbuf);
                    adis_driver.state             = ADIS_RX_PEND;
                    break;
                default:
                    break;
            }
            break;
        case ADIS_RX_PEND:
            // move tx data to cache
            for(i=0; i<adis_driver.current_tx_numbytes; ++i) {
                adis_cache_data.adis_tx_cache[i] = adis_driver.adis_txbuf[i];
            }
            for(i=0; i<adis_driver.current_rx_numbytes; ++i) {
                adis_cache_data.adis_rx_cache[i] = adis_driver.adis_rxbuf[i];
            }
            adis_cache_data.reg = adis_driver.reg;
            adis_cache_data.current_rx_numbytes = adis_driver.current_rx_numbytes;
            adis_cache_data.current_tx_numbytes = adis_driver.current_tx_numbytes;

            // broadcast event
            chEvtBroadcastI(&adis_newdata_event);
            adis_driver.state             = ADIS_IDLE;

            spiUnselect(adis_driver.spi_instance);                /* Slave Select de-assertion.       */
            spiReleaseBus(adis_driver.spi_instance);              /* Ownership release.               */
            break;
        default:
        	adis_driver.state             = ADIS_IDLE;
        	spiUnselect(adis_driver.spi_instance);                /* Slave Select de-assertion.       */
            spiReleaseBus(adis_driver.spi_instance);              /* Ownership release.               */
        	break;
    }
    chSysUnlockFromIsr();
}

/*! \brief Create a read address
 *
 * @param s
 * @return  formatted read address for adis
 */
static adis_regaddr adis_create_read_addr(adis_regaddr s) {
    return (s & 0b01111111);
}

/*! \brief Create a write address
 *
 * @param s
 * @return  formatted write address for adis
 */
static adis_regaddr adis_create_write_addr(adis_regaddr s) {
    return (s | 0b10000000);
}

///*! \brief write the smpl_prd register
// *
// * @param time_base    The time base parameter (0 or 1)
// * @param sample_prd   7 bit sample prd parameter
// */
//void adis_write_smpl_prd(uint8_t time_base, uint8_t sample_prd) {
//
//	/* tb is a 1 bit field */
//	uint8_t  tb = time_base & 0b01;
//	/* Ns is a 7 bit field */
//	uint8_t  Ns = sample_prd & 0b01111111;
//
//	spi_init_master_xact_data(&adis_read_smpl_prd_xact);
//
//	adis_read_smpl_prd_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
//	adis_read_smpl_prd_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
//	adis_read_smpl_prd_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;
//
//	adis_read_smpl_prd_xact.writebuf[0]     = adis_create_write_addr(ADIS_SMPL_PRD);
//	adis_read_smpl_prd_xact.writebuf[1]     = ((tb<<7) | Ns);
//	adis_read_smpl_prd_xact.write_numbytes  = 2;
//	adis_read_smpl_prd_xact.read_numbytes   = 0;
//	adis_read_smpl_prd_xact.dummy_value     = 0x7f;
//
//	// Start the transaction
//	start_spi_master_xact_intr(&adis_read_smpl_prd_xact, adis_write_cb) ;
//
//}
//

//void adis_read_brst_mode(SPI_XACT_FnCallback cb) {
//	bool success = false;
//
//	spi_init_master_xact_data(&adis_read_brst_mode_xact);
//
//	adis_read_brst_mode_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
//	adis_read_brst_mode_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
//	adis_read_brst_mode_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;
//
//	adis_read_brst_mode_xact.writebuf[0]     = adis_create_read_addr(ADIS_GLOB_CMD);
//	adis_read_brst_mode_xact.writebuf[1]     = 0x0;
//	adis_read_brst_mode_xact.write_numbytes  = 2;
//	adis_read_brst_mode_xact.read_numbytes   = (ADIS_NUM_BURSTREAD_REGS * 2);
//	adis_read_brst_mode_xact.dummy_value     = 0x7f;
//
//	// Start the transaction
//	success = start_spi_master_xact_intr(&adis_read_brst_mode_xact, cb) ;
//	if(!success) {
//		uart0_putstring("burst SPI fail\n");
//	}
//}

void adis_read_id(SPIDriver *spip) {
    adis_driver.spi_instance      = spip;
    adis_driver.adis_txbuf[0]     = adis_create_read_addr(ADIS_PRODUCT_ID);
    adis_driver.adis_txbuf[1]     = (adis_data) 0;
    adis_driver.reg               = ADIS_PRODUCT_ID;

    spiAcquireBus(spip);                /* Acquire ownership of the bus.    */
    spiStart(spip, &adis_spicfg);       /* Setup transfer parameters.       */
    spiSelect(spip);                    /* Slave Select assertion.          */
    spiStartSend(spip, 2, adis_driver.adis_txbuf);

    adis_driver.state             = ADIS_TX_PEND;
}




void spi_test(SPIDriver *spip) {
    adis_driver.spi_instance      = spip;
    adis_driver.adis_txbuf[0]     = adis_create_read_addr(ADIS_PRODUCT_ID);
    adis_driver.adis_txbuf[1]     = (adis_data) 0;
    adis_driver.reg               = ADIS_PRODUCT_ID;
    spiAcquireBus(spip);              /* Acquire ownership of the bus.    */

    spiStart(spip, &adis_spicfg);       /* Setup transfer parameters.       */
    spiSelect(spip);                  /* Slave Select assertion.          */
    spiExchange(spip, 10,
    		adis_driver.adis_txbuf, adis_driver.adis_rxbuf);          /* Atomic transfer operations.      */

	chThdSleepMilliseconds(30);

    spiUnselect(spip);                /* Slave Select de-assertion.       */
    spiReleaseBus(spip);              /* Ownership release.               */

    //adis_driver.state             = ADIS_TX_PEND;
}
/*!
 * @}
 */
