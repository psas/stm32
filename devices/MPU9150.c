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

#include "usbdetail.h"
#include "chprintf.h"

#include "MPU9150.h"

#if !defined(MPU9150_DEBUG) || defined(__DOXYGEN__)
#define 	MPU9150_DEBUG                   0
#endif

mpu9150_cache            mpu9150_cache_data;
mpu9150_a_g_burst_data   mpu9150_burst_a_g_data;
mpu9150_magn_burst_data  mpu9150_burst_magn_data;

MPU9150_Driver           mpu9150_driver;
EventSource              mpu9150_int_event;

const       systime_t    mpu9150_i2c_timeout        = MS2ST(400);
const       uint8_t      mpu9150_i2c_a_g_addr       = 0x68;    // See page 8 , MPU9150 Register Map and Descriptions r4.0
const       uint8_t      mpu9150_i2c_magn_addr      = 0x0C;    // See page 28, MPU9150 Product Specification r4.0

#if DEBUG_MPU9150 || defined(__DOXYGEN__)
static i2c_error_info i2c_debug_errors[] = {
    {"I2C_NO_ERROR    ",   0x00},
    {"I2C_BUS_ERROR   ",   0x01},
    {"I2C_ARBIT_LOST  ",   0x02},
    {"I2C_ACK_FAIL    ",   0x04},
    {"I2CD_ACK_FAILURE",   0x04},
    {"I2CD_OVERRUN    ",   0x08},
    {"I2CD_PEC_ERROR  ",   0x10},
    {"I2CD_TIMEOUT    ",   0x20},
    {"I2CD_SMB_ALERT  ",   0x40}
};

const char* i2c_errno_str(int32_t err) {
	uint8_t count_errors = sizeof(i2c_debug_errors)/sizeof(i2c_error_info);
	uint8_t i            = 0;

	for(i=0; i<count_errors; ++i) {
		if(i2c_debug_errors[i].error_number == err) {
			return i2c_debug_errors[i].err_string;
		}
	}
	return "I2C Error Unknown";
}

#endif

/*! mpu9150 interrupt event
 *
 * @param id
 */
void mpu9150_int_event_handler(eventid_t id) {
	(void) id;

}

/*! \brief Initialize MPU9150 driver
 *
 */
void mpu9150_init(I2CDriver* i2c) {
	uint8_t     i              = 0;

	mpu9150_driver.i2c_errors   = 0;
	mpu9150_driver.i2c_instance = i2c;
	for(i=0; i<MPU9150_MAX_TX_BUFFER; ++i) {
		mpu9150_driver.txbuf[i]        = 0;
		mpu9150_cache_data.tx_cache[i] = 0;
	}
	for(i=0; i<MPU9150_MAX_RX_BUFFER; ++i) {
		mpu9150_driver.rxbuf[i]        = 0xa5;
		mpu9150_cache_data.rx_cache[i] = 0xa5;
	}
	chEvtInit(&mpu9150_int_event);
}

/*! \read the id
 *
 */
void mpu9150_a_g_read_id(I2CDriver* i2cptr) {
	msg_t status = RDY_OK;

	mpu9150_driver.txbuf[0] = A_G_WHO_AM_I;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpu9150_i2c_a_g_addr, mpu9150_driver.txbuf, 1, mpu9150_driver.rxbuf, 1, mpu9150_i2c_timeout);
	i2cReleaseBus(i2cptr);

	if (status != RDY_OK){
		mpu9150_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
}



//! @}
