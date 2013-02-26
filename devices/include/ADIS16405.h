/*! \file ADIS16405.h
 *
 * Intended for use with ChibiOS RT
 */

/*! \addtogroup adis16405
 * @{
 */

#ifndef _ADIS16405_H
#define _ADIS16405_H


#ifdef __cplusplus
extern "C" {
#endif

#include "ch.h"
#include "hal.h"

#define     ADIS_MAX_TX                       64
#define     ADIS_MAX_RX                       32

#define     ADIS_RESET_MSECS                  500

#define     ADIS_MAX_RX_BUFFER                64
#define     ADIS_MAX_TX_BUFFER                16

#define     ADIS_NUM_BURSTREAD_REGS           12

typedef     uint8_t                           adis_data;

/*! \typedef adis_regaddr
 *
 * ADIS Register addresses
 *
 */
typedef enum {
	// Name         address         default    function
	ADIS_FLASH_CNT    = 0x00,        // N/A     Flash memory write count
	ADIS_SUPPLY_OUT   = 0x02,        // N/A     Power supply measurement
	ADIS_XGYRO_OUT    = 0x04,        // N/A     X-axis gyroscope output
	ADIS_YGYRO_OUT    = 0x06,        // N/A     Y-axis gyroscope output
	ADIS_ZGYRO_OUT    = 0x08,        // N/A     Z-axis gyroscope output
	ADIS_XACCL_OUT    = 0x0A,        // N/A     X-axis accelerometer output
	ADIS_YACCL_OUT    = 0x0C,        // N/A     Y-axis accelerometer output
	ADIS_ZACCL_OUT    = 0x0E,        // N/A     Z-axis accelerometer output
	ADIS_XMAGN_OUT    = 0x10,        // N/A     X-axis magnetometer measurement
	ADIS_YMAGN_OUT    = 0x12,        // N/A     Y-axis magnetometer measurement
	ADIS_ZMAGN_OUT    = 0x14,        // N/A     Z-axis magnetometer measurement
	ADIS_TEMP_OUT     = 0x16,        // N/A     Temperature output
	ADIS_AUX_ADC      = 0x18,        // N/A     Auxiliary ADC measurement
	ADIS_XGYRO_OFF    = 0x1A,        // 0x0000  X-axis gyroscope bias offset factor
	ADIS_YGYRO_OFF    = 0x1C,        // 0x0000  Y-axis gyroscope bias offset factor
	ADIS_ZGYRO_OFF    = 0x1E,        // 0x0000  Z-axis gyroscope bias offset factor
	ADIS_XACCL_OFF    = 0x20,        // 0x0000  X-axis acceleration bias offset factor
	ADIS_YACCL_OFF    = 0x22,        // 0x0000  Y-axis acceleration bias offset factor
	ADIS_ZACCL_OFF    = 0x24,        // 0x0000  Z-axis acceleration bias offset factor
	ADIS_XMAGN_HIF    = 0x26,        // 0x0000  X-axis magnetometer, hard-iron factor
	ADIS_YMAGN_HIF    = 0x28,        // 0x0000  Y-axis magnetometer, hard-iron factor
	ADIS_ZMAGN_HIF    = 0x2A,        // 0x0000  Z-axis magnetometer, hard-iron factor
	ADIS_XMAGN_SIF    = 0x2C,        // 0x0800  X-axis magnetometer, soft-iron factor
	ADIS_YMAGN_SIF    = 0x2E,        // 0x0800  Y-axis magnetometer, soft-iron factor
	ADIS_ZMAGN_SIF    = 0x30,        // 0x0800  Z-axis magnetometer, soft-iron factor
	ADIS_GPIO_CTRL    = 0x32,        // 0x0000  Auxiliary digital input/output control
	ADIS_MSC_CTRL     = 0x34,        // 0x0006  Miscellaneous control
	ADIS_SMPL_PRD     = 0x36,        // 0x0001  Internal sample period (rate) control
	ADIS_SENS_AVG     = 0x38,        // 0x0402  Dynamic range and digital filter control
	ADIS_SLP_CNT      = 0x3A,        // 0x0000  Sleep mode control
	ADIS_DIAG_STAT    = 0x3C,        // 0x0000  System status
	ADIS_GLOB_CMD     = 0x3E,        // 0x0000  System command
	ADIS_ALM_MAG1     = 0x40,        // 0x0000  Alarm 1 amplitude threshold
	ADIS_ALM_MAG2     = 0x42,        // 0x0000  Alarm spi_master_xact_data* caller, spi_master_xact_data* spi_xact, void* data2 amplitude threshold
	ADIS_ALM_SMPL1    = 0x44,        // 0x0000  Alarm 1 sample size
	ADIS_ALM_SMPL2    = 0x46,        // 0x0000  Alarm 2 sample size
	ADIS_ALM_CTRL     = 0x48,        // 0x0000  Alarm control
	ADIS_AUX_DAC      = 0x4A,        // 0x0000  Auxiliary DAC data
	//          =0x4C,to 0x55    //         Reserved
	ADIS_PRODUCT_ID   = 0x56        //         Product identifier

} adis_regaddr;

typedef enum {
	ADIS_OFF,
	ADIS_IDLE,
	ADIS_TX_PEND,
	ADIS_RX_PEND,
	ADIS_ERROR
} adis_xact_state;

/*!
 * Structure for keeping track of an ADIS transaction
 *
 * The SPI transactions are implemented with asynchronous
 * (interrupt driven) ChibiOS SPI APIs.
 *
 */
typedef struct {
	uint8_t            rx_numbytes;                    /*! number of bytes to receive in this current transaction  */
	uint8_t            tx_numbytes;                    /*! number of bytes to transmit in this current transaction */
	SPIDriver*         spi_instance;                   /*! which stm32f407 SPI instance to use (there are 3)       */
	adis_xact_state    state;                          /*! Current state of the ADIS transaction                   */
	adis_regaddr       reg;                            /*! Register address in this ADIS transaction               */
	adis_data          adis_txbuf[ADIS_MAX_TX_BUFFER]; /*! Transmit buffer                                         */
	adis_data          adis_rxbuf[ADIS_MAX_RX_BUFFER]; /*! Receive buffer                                          */
	uint8_t            debug_cb_count;
} ADIS_Driver;

/*!
 * Another transaction may begin which would corrupt the tx and rx
 * buffers. Copy the SPI buffers to the cache for reading in
 * mainline after event detection.
 */
typedef struct {
	uint8_t            current_rx_numbytes;
	uint8_t            current_tx_numbytes;
	adis_regaddr       reg;
	adis_data          adis_tx_cache[ADIS_MAX_TX_BUFFER];
	adis_data          adis_rx_cache[ADIS_MAX_RX_BUFFER];
} adis_cache;

/*! \typedef adis_config
 *
 * Configuration for the ADIS connections EXCEPT SPI
 */
typedef struct {
	/*! \brief The reset line port
	 */
	ioportid_t               reset_port;
	/*! \brief The reset line pad number.
	 */
	uint16_t                 reset_pad;
	/*! \brief The DIO1 port
	 */
	ioportid_t               dio1_port;
	/*! \brief The DIO1 pad number.
	 */
	uint16_t                 dio1_pad;
	/*! \brief The DIO2 port
	 */
	ioportid_t               dio2_port;
	/*! \brief The DIO2 pad number.
	 */
	uint16_t                 dio2_pad;
	/*! \brief The DIO3 port
	 */
	ioportid_t               dio3_port;
	/*! \brief The DIO3 pad number.
	 */
	uint16_t                 dio3_pad;
	/*! \brief The DIO4 port
	 */
	ioportid_t               dio4_port;
	/*! \brief The DIO4 pad number.
	 */
	uint16_t                 dio4_pad;

} adis_connect;


extern const SPIConfig      adis_spicfg ;
extern const adis_connect   adis_connections ;
extern       adis_cache     adis_cache_data;
extern       EventSource    adis_spi_cb_event;

extern       ADIS_Driver    adis_driver;

void     adis_init(void);
void     adis_reset(void);

void     adis_spi_cb(SPIDriver *spip) ;

void     adis_write_smpl_prd(uint8_t time_base, uint8_t sample_prd);
void     adis_read_brst_mode(void);
void     adis_read_id(SPIDriver *spip);
void     spi_test(SPIDriver *spip);

/*!
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif


