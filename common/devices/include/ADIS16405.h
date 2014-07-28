/* Intended for use with ChibiOS RT */

#ifndef _ADIS16405_H
#define _ADIS16405_H

#include "ch.h"
#include "hal.h"

#include "utils_hal.h"

/* ADIS Register addresses */
typedef enum {
	// Name         address         default    function
	ADIS_FLASH_CNT    = 0x00,        //  N/A     Flash memory write count
	ADIS_SUPPLY_OUT   = 0x02,        //  N/A     Power supply measurement
	ADIS_XGYRO_OUT    = 0x04,        //  N/A     X-axis gyroscope output
	ADIS_YGYRO_OUT    = 0x06,        //  N/A     Y-axis gyroscope output
	ADIS_ZGYRO_OUT    = 0x08,        //  N/A     Z-axis gyroscope output
	ADIS_XACCL_OUT    = 0x0A,        //  N/A     X-axis accelerometer output
	ADIS_YACCL_OUT    = 0x0C,        //  N/A     Y-axis accelerometer output
	ADIS_ZACCL_OUT    = 0x0E,        //  N/A     Z-axis accelerometer output
	ADIS_XMAGN_OUT    = 0x10,        //  N/A     X-axis magnetometer measurement
	ADIS_YMAGN_OUT    = 0x12,        //  N/A     Y-axis magnetometer measurement
	ADIS_ZMAGN_OUT    = 0x14,        //  N/A     Z-axis magnetometer measurement
	ADIS_TEMP_OUT     = 0x16,        //  N/A     Temperature output
	ADIS_AUX_ADC      = 0x18,        //  N/A     Auxiliary ADC measurement
	ADIS_XGYRO_OFF    = 0x1A,        //  0x0000  X-axis gyroscope bias offset factor
	ADIS_YGYRO_OFF    = 0x1C,        //  0x0000  Y-axis gyroscope bias offset factor
	ADIS_ZGYRO_OFF    = 0x1E,        //  0x0000  Z-axis gyroscope bias offset factor
	ADIS_XACCL_OFF    = 0x20,        //  0x0000  X-axis acceleration bias offset factor
	ADIS_YACCL_OFF    = 0x22,        //  0x0000  Y-axis acceleration bias offset factor
	ADIS_ZACCL_OFF    = 0x24,        //  0x0000  Z-axis acceleration bias offset factor
	ADIS_XMAGN_HIF    = 0x26,        //  0x0000  X-axis magnetometer, hard-iron factor
	ADIS_YMAGN_HIF    = 0x28,        //  0x0000  Y-axis magnetometer, hard-iron factor
	ADIS_ZMAGN_HIF    = 0x2A,        //  0x0000  Z-axis magnetometer, hard-iron factor
	ADIS_XMAGN_SIF    = 0x2C,        //  0x0800  X-axis magnetometer, soft-iron factor
	ADIS_YMAGN_SIF    = 0x2E,        //  0x0800  Y-axis magnetometer, soft-iron factor
	ADIS_ZMAGN_SIF    = 0x30,        //  0x0800  Z-axis magnetometer, soft-iron factor
	ADIS_GPIO_CTRL    = 0x32,        //  0x0000  Auxiliary digital input/output control
	ADIS_MSC_CTRL     = 0x34,        //  0x0006  Miscellaneous control
	ADIS_SMPL_PRD     = 0x36,        //  0x0001  Internal sample period (rate) control
	ADIS_SENS_AVG     = 0x38,        //  0x0402  Dynamic range and digital filter control
	ADIS_SLP_CNT      = 0x3A,        //  0x0000  Sleep mode control
	ADIS_DIAG_STAT    = 0x3C,        //  0x0000  System status
	ADIS_GLOB_CMD     = 0x3E,        //  0x0000  System command
	ADIS_ALM_MAG1     = 0x40,        //  0x0000  Alarm 1 amplitude threshold
	ADIS_ALM_MAG2     = 0x42,        //  0x0000  Alarm spi_master_xact_data* caller, spi_master_xact_data* spi_xact, void* data2 amplitude threshold
	ADIS_ALM_SMPL1    = 0x44,        //  0x0000  Alarm 1 sample size
	ADIS_ALM_SMPL2    = 0x46,        //  0x0000  Alarm 2 sample size
	ADIS_ALM_CTRL     = 0x48,        //  0x0000  Alarm control
	ADIS_AUX_DAC      = 0x4A,        //  0x0000  Auxiliary DAC data
	//                = 0x4C to 0x55 //          Reserved
	ADIS_PRODUCT_ID   = 0x56         //          Product identifier

} adis_regaddr;

/*! \typedef
 * Burst data collection. This establishes what we consider the right datatype
 * for the registers because trying to work with 12 or 14 bit twos complement
 * that doesn't sign extend to 16 bits is unpleasant.
 */
typedef struct ADIS16405Data {
	uint16_t supply_out;//  Power supply measurement
	int16_t xgyro_out;  //  X-axis gyroscope output
	int16_t ygyro_out;  //  Y-axis gyroscope output
	int16_t zgyro_out;  //  Z-axis gyroscope output
	int16_t xaccl_out;  //  X-axis accelerometer output
	int16_t yaccl_out;  //  Y-axis accelerometer output
	int16_t zaccl_out;  //  Z-axis accelerometer output
	int16_t xmagn_out;  //  X-axis magnetometer measurement
	int16_t ymagn_out;  //  Y-axis magnetometer measurement
	int16_t zmagn_out;  //  Z-axis magnetometer measurement
	int16_t temp_out;   //  Temperature output
	uint16_t aux_adc;   //  Auxiliary ADC measurement
} ADIS16405Data;

/*! \typedef adis_config
 *
 * Configuration for the ADIS connections
 */

typedef struct {
	struct pin spi_sck;  /*! \brief The SPI SCK wire */
	struct pin spi_miso; /*! \brief The SPI MISO wire */
	struct pin spi_mosi; /*! \brief The SPI MOSI wire */
	struct pin spi_cs;   /*! \brief The SPI CS wire */
	SPIDriver *SPID;     /*! \brief the SPI driver */
	struct pin reset;    /*! \brief The reset wire */
	struct pin dio1;     /*! \brief The DIO1 wire */
	struct pin dio2;     /*! \brief The DIO2 wire */
	struct pin dio3;     /*! \brief The DIO3 wire */
	struct pin dio4;     /*! \brief The DIO4 wire */
} ADIS16405Config;

extern const ADIS16405Config adis_olimex_e407;

extern EventSource ADIS16405_data_ready;

void adis_init(const ADIS16405Config * conf);
uint16_t adis_get(adis_regaddr addr);
void adis_set(adis_regaddr addr, uint16_t value);
void adis_get_data(ADIS16405Data * data);
void adis_reset(void);
uint16_t adis_self_test(void);

#endif


