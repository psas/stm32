
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "usbdetail.h"
#include "chprintf.h"
#include "extdetail.h"
#include "MPL3115A2.h"



#if !defined(MPL3115A2_DEBUG) || defined(__DOXYGEN__)
#define 	MPL3115A2_DEBUG                   0
#endif

mpl3115a2_cache            mpl3115a2_cache_data;
mpl3115a2_a_g_burst_data   mpl3115a2_burst_a_g_data;
mpl3115a2_magn_burst_data  mpl3115a2_burst_magn_data;

MPL3115A2_Driver           mpl3115a2_driver;
EventSource              mpl3115a2_int_event;

const       systime_t    mpl3115a2_i2c_timeout        = MS2ST(400);
//const       uint8_t      mpl3115a2_i2c_a_g_addr       = 0x68;    // See page 8 , MPL3115A2 Register Map and Descriptions r4.0
//const       uint8_t      mpl3115a2_i2c_a_g_addr       = 0x0C;
const		uint8_t	mpl3115a2_i2c_slave_addr	= 0x60;
const       uint8_t      mpl3115a2_i2c_magn_addr      = 0x0C;    // See page 28, MPL3115A2 Product Specification r4.0

#if DEBUG_MPL3115A2 || defined(__DOXYGEN__)
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

/*! mpl3115a2 interrupt event
 *
 * @param id
 */
void mpl3115a2_int_event_handler(eventid_t id) {
	(void) id;

}

void mpl3115a2_start(I2CDriver* i2c) {
	uint8_t     i              = 0;

	mpl3115a2_driver.i2c_errors   = 0;
	mpl3115a2_driver.i2c_instance = i2c;
	for(i=0; i<MPL3115A2_MAX_TX_BUFFER; ++i) {
		mpl3115a2_driver.txbuf[i]        = 0;
		mpl3115a2_cache_data.tx_cache[i] = 0;
	}
	for(i=0; i<MPL3115A2_MAX_RX_BUFFER; ++i) {
		mpl3115a2_driver.rxbuf[i]        = 0xa5;
		mpl3115a2_cache_data.rx_cache[i] = 0xa5;
	}
	//chEvtInit(&mpu9150_int_event);
}

msg_t mpl3115a2_init(I2CDriver* i2cptr) {
	//configure the altimiter
	msg_t status = RDY_OK;
	//enable altimter mode
	mpl3115a2_driver.txbuf[0] = CTRL_REG1;
	mpl3115a2_driver.txbuf[1] = 0xB9; 
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	//turn on data ready interrupt
	mpl3115a2_driver.txbuf[0] = CTRL_REG4;
	mpl3115a2_driver.txbuf[1] = 0x80;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	//enable pressure and temp event flags
	mpl3115a2_driver.txbuf[0] = PT_DATA_CFG;
	mpl3115a2_driver.txbuf[0] = 0x07;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	//calibrate sea level air pressure (default for now, we may want to change)
	//this is set in units of 2 pa
	mpl3115a2_driver.txbuf[0] = BAR_IN_LSB;
	mpl3115a2_driver.txbuf[1] = 0x80;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	mpl3115a2_driver.txbuf[0] = BAR_IN_MSB;
	mpl3115a2_driver.txbuf[1] = 0x8D;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	return status;
}

float mpl3115a2_get_altitude (I2CDriver* i2cptr) {
	//get the altitude
	float altitude = 5.0;
	uint8_t m_altitude;
	uint8_t c_altitude;
	float l_altitude;
	msg_t status = RDY_OK;
	//get the most significant bits of the altitude (integer part)
	mpl3115a2_driver.txbuf[0] = OUT_P_MSB;	
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	/*
	m_altitude = mpl3115a2_driver.rxbuf[0];
	//get the middle significant bits of the altitude (integer part)
	mpl3115a2_driver.txbuf[0] = OUT_P_CSB;	
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	c_altitude = mpl3115a2_driver.rxbuf[0];
	//get the least significant bits of the altitude
	mpl3115a2_driver.txbuf[0] = OUT_P_LSB;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	l_altitude = (float) (mpl3115a2_driver.rxbuf[0]>>4)/16.0;
	altitude = (float)((m_altitude << 8)|c_altitude) + l_altitude;
	*/
	return altitude;
}

float mpl3115a2_get_temperature (I2CDriver* i2cptr) {
	float temp = 0.0;
	uint8_t m_temp;
	float l_temp;
	msg_t status = RDY_OK;
	/*
	mpl3115a2_driver.txbuf[0] = OUT_T_MSB;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	*/
		mpl3115a2_driver.txbuf[0] = CTRL_REG1;
	mpl3115a2_driver.txbuf[1] = 0xB9; 
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	/*
	m_temp = mpl3115a2_driver.rxbuf[0];
	mpl3115a2_driver.txbuf[0] = OUT_T_LSB;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr,mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1 , mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);
	l_temp = (float) (mpl3115a2_driver.rxbuf[0] >> 4)/16.0;
	temp = (float) (m_temp + l_temp);
	*/
	return temp;
}

/*! \brief Initialize MPL3115A2 driver
 *
 */

/*
void mpl3115a2_start(I2CDriver* i2c) {
	uint8_t     i              = 0;

	mpl3115a2_driver.i2c_errors   = 0;
	mpl3115a2_driver.i2c_instance = i2c;
	for(i=0; i<MPL3115A2_MAX_TX_BUFFER; ++i) {
		mpl3115a2_driver.txbuf[i]        = 0;
		mpl3115a2_cache_data.tx_cache[i] = 0;
	}
	for(i=0; i<MPL3115A2_MAX_RX_BUFFER; ++i) {
		mpl3115a2_driver.rxbuf[i]        = 0xa5;
		mpl3115a2_cache_data.rx_cache[i] = 0xa5;
	}
	chEvtInit(&mpl3115a2_int_event);
}

*/

/*

void mpl3115a2_write_pm1(I2CDriver* i2cptr, mpl3115a2_reg_data d) {
	msg_t status = RDY_OK;

	//mpl3115a2_driver.txbuf[0] = A_G_PWR_MGMT_1;
	mpl3115a2_driver.txbuf[0] = 0;
	mpl3115a2_driver.txbuf[1] = d;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_a_g_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1, mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);

	if (status != RDY_OK){
		mpl3115a2_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
}

*/

/*! \brief Initialize mpl3115a2
 *
 */

/*
void mpl3115a2_init(I2CDriver* i2cptr) {
	mpl3115a2_reg_data           rdata;

	rdata = MPL3115A2_X_GYRO_CLOCKREF;
    mpl3115a2_write_pm1(i2cptr, rdata);

}

*/

/*! \read the id
 *
 */

/*
void mpl3115a2_a_g_read_id(I2CDriver* i2cptr) {
	msg_t status = RDY_OK;

	mpl3115a2_driver.txbuf[0] = WHO_AM_I;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_a_g_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1, mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);

	if (status != RDY_OK){
		mpl3115a2_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
}


*/

/*
void mpl3115a2_setup(I2CDriver* i2cptr) {
	msg_t status = RDY_OK;

	//mpl3115a2_driver.txbuf[0] = A_G_INT_PIN_CFG;
	//mpl3115a2_driver.txbuf[1] = MPL3115A2_I2C_BYPASS | MPL3115A2_INT_LEVEL;
	mpl3115a2_driver.txbuf[0] = CTRL_REG1;
	mpl3115a2_driver.txbuf[1] = 0xB9;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_a_g_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0, mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);

	if (status != RDY_OK){
		mpl3115a2_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
}

*/


/*
void mpl3115a2_test(I2CDriver* i2cptr) {
	msg_t status = RDY_OK;

	//mpl3115a2_driver.txbuf[0] = A_G_USER_CTRL;
	mpl3115a2_driver.txbuf[0] = 0;	
	//mpl3115a2_driver.txbuf[1] = MPL3115A2_I2C_BYPASS | MPL3115A2_INT_LEVEL;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_a_g_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1, mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);

	if (status != RDY_OK){
		mpl3115a2_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
}

*/
/*! \read the id
 *
 */

/*
void mpl3115a2_magn_read_id(I2CDriver* i2cptr) {
	msg_t status = RDY_OK;

	mpl3115a2_driver.txbuf[0] = MAGN_DEVICE_ID;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_magn_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1, mpl3115a2_i2c_timeout);
	i2cReleaseBus(i2cptr);

	if (status != RDY_OK){
		mpl3115a2_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
}
*/
//! @}


