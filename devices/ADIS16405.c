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

#include "ADIS16405.h"

adis_cache                adis_data_cache;

/*! \brief Reset the ADIS
 */
void adis_reset() {

	ADIS_RESET_LOW;

	util_wait_msecs(ADIS_RESET_MSECS);

	ADIS_RESET_HIGH;

	util_wait_msecs(ADIS_RESET_MSECS);
}

bool adis_read_cache(adis_regaddr adis_recent, uint16_t* adis_data) {

	adis_cache*       c;
	c               = &adis_data_cache;

	if((adis_recent== ADIS_PRODUCT_ID)) {
		adis_data[0] = ( (c->adis_prod_id.data_high << 8) |  c->adis_prod_id.data_low);
		return true;
	}
	if((adis_recent== ADIS_SMPL_PRD)) {
		adis_data[0] = ( (c->adis_sampl_per.data_high << 8) |  c->adis_sampl_per.data_low);
		return true;
	}
	return false;
}

void adis_get_data() {

	bool            ok            = false;

	adis_regaddr    adis_recent  = 0;

	uint16_t        adis_data[ADIS_MAX_DATA_BUFFER];

	ok = rb_get_elem((uint8_t *) &adis_recent, &adis_spi_done_q);
	if(ok) {
		ok = adis_read_cache(adis_recent, adis_data);
		if(ok) {
			switch(adis_recent) {

			case ADIS_PRODUCT_ID:
				printf_lpc(UART0, "ID 0x%x\r\n",      adis_data[0]) ;
				break;
			case ADIS_SMPL_PRD:
				printf_lpc(UART0, "SMPL_PER 0x%x\r\n",      adis_data[0]) ;
				break;
			default:
				break;

			}
		}
	}
}

void adis_write_cb(spi_master_xact_data* caller, spi_master_xact_data* spi_xact, void* data) {

}


void adis_read_cb(spi_master_xact_data* caller, spi_master_xact_data* spi_xact, void* data) {

	uint16_t i             = 0;
	adis_regaddr  regaddr  = 0;

	// copy out read buffer.
	for(i=0; i<spi_xact->read_numbytes; ++i) {
		caller->readbuf[i] = spi_xact->readbuf[i];
	}
	// copy out write buffer.
	for(i=0; i<spi_xact->write_numbytes; ++i) {
		caller->writebuf[i] = spi_xact->writebuf[i];
	}

	// The register address is always the first byte of the write buffer.
	regaddr = caller->writebuf[0];

	switch(regaddr) {
		case ADIS_PRODUCT_ID:
			adis_data_cache.adis_prod_id.data_high   = caller->readbuf[1];
			adis_data_cache.adis_prod_id.data_low    = caller->readbuf[2];

			break;
		case ADIS_SMPL_PRD:
			adis_data_cache.adis_sampl_per.data_high  = caller->readbuf[1];
			adis_data_cache.adis_sampl_per.data_low   = caller->readbuf[2];
			break;
		default:
			break;
	}
	if(!rb_is_full(&adis_spi_done_q)) {
		rb_put_elem((char) regaddr, &adis_spi_done_q);
	}       // now check queue not empty in main loop to see if there is fresh data.
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

/*! \brief write the smpl_prd register
 *
 * @param time_base    The time base parameter (0 or 1)
 * @param sample_prd   7 bit sample prd parameter
 */
void adis_write_smpl_prd(uint8_t time_base, uint8_t sample_prd) {

	/* tb is a 1 bit field */
	uint8_t  tb = time_base & 0b01;
	/* Ns is a 7 bit field */
	uint8_t  Ns = sample_prd & 0b01111111;

	spi_init_master_xact_data(&adis_read_smpl_prd_xact);

	adis_read_smpl_prd_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
	adis_read_smpl_prd_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
	adis_read_smpl_prd_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;

	adis_read_smpl_prd_xact.writebuf[0]     = adis_create_write_addr(ADIS_SMPL_PRD);
	adis_read_smpl_prd_xact.writebuf[1]     = ((tb<<7) | Ns);
	adis_read_smpl_prd_xact.write_numbytes  = 2;
	adis_read_smpl_prd_xact.read_numbytes   = 0;
	adis_read_smpl_prd_xact.dummy_value     = 0x7f;

	// Start the transaction
	start_spi_master_xact_intr(&adis_read_smpl_prd_xact, adis_write_cb) ;

}


void adis_read_brst_mode(SPI_XACT_FnCallback cb) {
	bool success = false;

	spi_init_master_xact_data(&adis_read_brst_mode_xact);

	adis_read_brst_mode_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
	adis_read_brst_mode_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
	adis_read_brst_mode_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;

	adis_read_brst_mode_xact.writebuf[0]     = adis_create_read_addr(ADIS_GLOB_CMD);
	adis_read_brst_mode_xact.writebuf[1]     = 0x0;
	adis_read_brst_mode_xact.write_numbytes  = 2;
	adis_read_brst_mode_xact.read_numbytes   = (ADIS_NUM_BURSTREAD_REGS * 2);
	adis_read_brst_mode_xact.dummy_value     = 0x7f;

	// Start the transaction
	success = start_spi_master_xact_intr(&adis_read_brst_mode_xact, cb) ;
	if(!success) {
		uart0_putstring("burst SPI fail\n");
	}
}


void adis_read_smpl_prd() {

	spi_init_master_xact_data(&adis_read_smpl_prd_xact);

	adis_read_smpl_prd_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
	adis_read_smpl_prd_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
	adis_read_smpl_prd_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;

	adis_read_smpl_prd_xact.writebuf[0]     = adis_create_read_addr(ADIS_SMPL_PRD);
	adis_read_smpl_prd_xact.writebuf[1]     = 0x0;
	adis_read_smpl_prd_xact.write_numbytes  = 2;
	adis_read_smpl_prd_xact.read_numbytes   = 2;
	adis_read_smpl_prd_xact.dummy_value     = 0x7f;

	// Start the transaction
	start_spi_master_xact_intr(&adis_read_smpl_prd_xact, adis_read_cb) ;

}

void adis_read_id() {

	spi_init_master_xact_data(&adis_read_id_xact);

	adis_read_id_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
	adis_read_id_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
	adis_read_id_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;

	adis_read_id_xact.writebuf[0]     = adis_create_read_addr(ADIS_PRODUCT_ID);
	adis_read_id_xact.writebuf[1]     = 0x0;
	adis_read_id_xact.write_numbytes  = 2;
	adis_read_id_xact.read_numbytes   = 2;
	adis_read_id_xact.dummy_value     = 0x7f;

	// Start the transaction
	start_spi_master_xact_intr(&adis_read_id_xact, adis_read_cb) ;
}

void adis_read_gpio_ctl() {

	spi_init_master_xact_data(&adis_read_id_xact);

	adis_read_gpio_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
	adis_read_gpio_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
	adis_read_gpio_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;

	adis_read_gpio_xact.writebuf[0]     = adis_create_read_addr(ADIS_GPIO_CTRL);
	adis_read_gpio_xact.writebuf[1]     = 0x0;
	adis_read_gpio_xact.write_numbytes  = 2;
	adis_read_gpio_xact.read_numbytes   = 2;
	adis_read_gpio_xact.dummy_value     = 0x7f;

	// Start the transaction
	start_spi_master_xact_intr(&adis_read_gpio_xact, adis_read_cb) ;
}

void adis_write_gpio_ctl() {

	spi_init_master_xact_data(&adis_read_id_xact);

	adis_write_gpio_xact.spi_cpha_val    = SPI_SCK_SECOND_CLK;
	adis_write_gpio_xact.spi_cpol_val    = SPI_SCK_ACTIVE_HIGH;
	adis_write_gpio_xact.spi_lsbf_val    = SPI_DATA_MSB_FIRST;

	adis_write_gpio_xact.writebuf[0]     = adis_create_write_addr(ADIS_GPIO_CTRL);
	printf_lpc(UART0, "read addr is: 0x%x\n", adis_write_gpio_xact.writebuf[0]);
	adis_write_gpio_xact.writebuf[1]     = 0x0;
	adis_write_gpio_xact.write_numbytes  = 2;
	adis_write_gpio_xact.read_numbytes   = 0;
	adis_write_gpio_xact.dummy_value     = 0x7f;

	// Start the transaction
	start_spi_master_xact_intr(&adis_write_gpio_xact, adis_read_cb) ;
}

/*!
 * @}
 */
