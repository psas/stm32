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

ADIS_Driver        adis_driver;

EventSource        adis_spi_cb_event;

adis_cache         adis_cache_data;


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
    adis_driver.debug_cb_count = 0;

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

	++adis_driver.debug_cb_count;

	uint8_t       i                              = 0;

	chDbgCheck(adis_driver.spi_instance == spip, "adis_spi_cb driver mismatch");

	switch(adis_driver.state) {
		case ADIS_TX_PEND:
			chEvtBroadcastI(&adis_spi_cb_event);
			break;
		case ADIS_RX_PEND:
			for(i=0; i<adis_driver.tx_numbytes; ++i) {
				adis_cache_data.adis_tx_cache[i] = adis_driver.adis_txbuf[i];
			}
			for(i=0; i<adis_driver.rx_numbytes; ++i) {
				adis_cache_data.adis_rx_cache[i] = adis_driver.adis_rxbuf[i];
			}
			adis_cache_data.reg                  = adis_driver.reg;
			adis_cache_data.current_rx_numbytes  = adis_driver.rx_numbytes;
			adis_cache_data.current_tx_numbytes  = adis_driver.tx_numbytes;

			chEvtBroadcastI(&adis_spi_cb_event);
			break;
		default:
			adis_driver.state                    = ADIS_IDLE;
			chEvtBroadcastI(&adis_spi_cb_event);
		//	spiUnselect(spip);                  //! \warning don't execute these things inside interrupt!!!!
		//	spiReleaseBus(spip);
			break;
	}
    chSysUnlockFromIsr();
}


/*! \brief Create a read address
 *
 * @param s
 * @return  formatted read address for adis
 */
static uint8_t adis_create_read_addr(adis_regaddr s) {
    return (s & 0b01111111);
}

/*! \brief Create a write address
 *
 * @param s
 * @return  formatted write address for adis
 */
//static adis_regaddr adis_create_write_addr(adis_regaddr s) {
//    return (s | 0b10000000);
//}


void adis_read_id(SPIDriver *spip) {
    adis_driver.spi_instance        = spip;
    adis_driver.adis_txbuf[0]       = adis_create_read_addr(ADIS_PRODUCT_ID);
    adis_driver.adis_txbuf[1]       = (adis_data) 0x0;
    adis_driver.adis_txbuf[2]       = (adis_data) 0x0;
    adis_driver.reg                 = ADIS_PRODUCT_ID;
    adis_driver.rx_numbytes         = 2;
    adis_driver.tx_numbytes         = 2;
    adis_driver.debug_cb_count      = 0;

    spiAcquireBus(spip);                /* Acquire ownership of the bus.    */
    spiSelect(spip);                    /* Slave Select assertion.          */
    spiStartSend(spip, adis_driver.tx_numbytes, adis_driver.adis_txbuf);
    adis_driver.state             = ADIS_TX_PEND;
}


/*!
 * @}
 */
