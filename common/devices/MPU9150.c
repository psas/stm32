/*! \file MPU9150.c
 *
 * API to support reading sensor data from an MPU9150 IMU via I2C.
 *
 * The MPU9150 is an MPU6050 & AKM8975C in the same package with some extra
 * stuff. For this driver the AKM is accessed only in the 9150's master mode.
 */

#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "usbdetail.h"
#include "chprintf.h"

#include "MPU9150.h"


const char               mpuid[(sizeof("MPU9")-1)]  = "MPU9";

MPU9150_Driver           mpu9150_driver;
MPU9150_read_data        mpu9150_current_read;

EventSource              mpu9150_int_event;
EventSource              mpu9150_data_event;

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

static void MPU9150_get(I2CDriver* i2cptr, mpu9150_a_g_regaddr ra, mpu9150_reg_data* d) {
	msg_t status = RDY_OK;

	mpu9150_driver.txbuf[0] = ra;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpu9150_i2c_a_g_addr, mpu9150_driver.txbuf, 1, mpu9150_driver.rxbuf, 1, mpu9150_i2c_timeout);
	i2cReleaseBus(i2cptr);
	if (status != RDY_OK){
		mpu9150_driver.i2c_errors = i2cGetErrors(i2cptr);
	}
	*d = mpu9150_driver.rxbuf[0];

    if (status != RDY_OK){
        mpu9150_driver.i2c_errors = i2cGetErrors(i2cptr);
    }
}

static void MPU9150_set(I2CDriver* i2cptr, mpu9150_a_g_regaddr ra, mpu9150_reg_data d) {
	msg_t status = RDY_OK;

	mpu9150_driver.txbuf[0] = ra;
	mpu9150_driver.txbuf[1] = d;
	i2cAcquireBus(i2cptr);
	status = i2cMasterTransmitTimeout(i2cptr, mpu9150_i2c_a_g_addr, mpu9150_driver.txbuf, 2, mpu9150_driver.rxbuf, 0, mpu9150_i2c_timeout);
	i2cReleaseBus(i2cptr);
    if (status != RDY_OK){
        mpu9150_driver.i2c_errors = i2cGetErrors(i2cptr);
    }
}

	return status;
}

/*! \brief Initialize MPU9150 driver
 *
 */
void mpu9150_start(I2CDriver* i2c) {
	uint8_t     i              = 0;

	mpu9150_driver.i2c_errors   = 0;
	mpu9150_driver.i2c_instance = i2c;
	for(i=0; i<MPU9150_MAX_TX_BUFFER; ++i) {
		mpu9150_driver.txbuf[i]        = 0;
	}
	for(i=0; i<MPU9150_MAX_RX_BUFFER; ++i) {
		mpu9150_driver.rxbuf[i]        = 0xa5;
	}
	mpu9150_current_read.accel_xyz.x     = 0;
	mpu9150_current_read.accel_xyz.y     = 0;
	mpu9150_current_read.accel_xyz.z     = 0;
	mpu9150_current_read.gyro_xyz.x      = 0;
	mpu9150_current_read.gyro_xyz.y      = 0;
	mpu9150_current_read.gyro_xyz.z      = 0;
	mpu9150_current_read.celsius         = 0;

	chEvtInit(&mpu9150_int_event);
}

void mpu9150_reset(I2CDriver* i2cptr) {
	/*! Turn on power */
	MPU9150_set(i2cptr, A_G_PWR_MGMT_1, MPU9150_PM1_RESET);

	chThdSleepMilliseconds(200);  // wait for device reset
	MPU9150_set(i2cptr, A_G_SIGNAL_PATH_RESET, 0b111);

	chThdSleepMilliseconds(200);  // wait for signal path reset
}

void MPU9150_set_pm1(I2CDriver* i2cptr, mpu9150_reg_data d) {
	/*! Turn on power */
    MPU9150_set(i2cptr, A_G_PWR_MGMT_1, &d);
}

void MPU9150_set_pin_cfg(I2CDriver* i2cptr, mpu9150_reg_data d) {
	MPU9150_set(i2cptr, A_G_INT_PIN_CFG, &d);
}

void MPU9150_set_int_enable(I2CDriver* i2cptr, mpu9150_reg_data d) {
	MPU9150_set(i2cptr, A_G_INT_ENABLE, &d);
}

void MPU9150_set_accel_config(I2CDriver* i2cptr, mpu9150_reg_data d) {
	MPU9150_set(i2cptr, A_G_ACCEL_CONFIG, &d);
}

void MPU9150_set_gyro_sample_rate_div(I2CDriver* i2cptr, mpu9150_reg_data d) {
	MPU9150_set(i2cptr, A_G_SMPLRT_DIV, &d);
}

void MPU9150_set_gyro_config(I2CDriver* i2cptr, mpu9150_reg_data d) {
	MPU9150_set(i2cptr, A_G_GYRO_CONFIG, &d);
}

void MPU9150_set_fifo_en(I2CDriver* i2cptr, mpu9150_reg_data d) {
	MPU9150_set(i2cptr, A_G_FIFO_EN, &d);
}

/*! \brief read the accel-gyro id
 *
 */
void mpu9150_a_g_read_id(I2CDriver* i2cptr) {
    mpu9150_reg_data d;
	MPU9150_get(i2cptr, A_G_WHO_AM_I, &d);
}

/*! \brief Convert register value to degrees C
 *
 * @param raw_temp
 * @return
 */
int16_t mpu9150_temp_to_dC(int16_t raw_temp) {
	return(raw_temp/340 + 35);
}

/*! \brief read the temperature register
 *
 */
int16_t mpu9150_a_g_read_temperature(I2CDriver* i2cptr) {
    mpu9150_reg_data d;

	int16_t      raw_temp = 0;

	MPU9150_get(i2cptr,A_G_TEMP_OUT_H, &d);
	raw_temp = mpu9150_driver.rxbuf[0] << 8;

    MPU9150_get(i2cptr, A_G_TEMP_OUT_L, &d);
	raw_temp = raw_temp | mpu9150_driver.rxbuf[0];

	return raw_temp;
}

/*! \brief read the interrupt status register
 *
 * Clears interrupt bits
 */
void mpu9150_a_g_read_int_status(I2CDriver* i2cptr) {
    mpu9150_reg_data d;
    MPU9150_get(i2cptr, A_G_INT_STATUS, &d);
}

/*! \brief read the accel-gyro fifo count
 *
 */
uint16_t mpu9150_a_g_fifo_cnt(I2CDriver* i2cptr) {
    mpu9150_reg_data d;
	uint16_t fifo_count = 0;

    MPU9150_get(i2cptr, A_G_FIFO_COUNTH, &d);
	fifo_count          = mpu9150_driver.rxbuf[0] << 8;

    MPU9150_get(i2cptr, A_G_FIFO_COUNTL, &d);
	fifo_count          = fifo_count | (mpu9150_driver.rxbuf[0] << 8);

	return fifo_count;
}

/*! \brief read the accel x,y,z
 *
 */
void mpu9150_a_read_x_y_z(I2CDriver* i2cptr, MPU9150_accel_data* d) {
	mpu9150_reg_data rdata = 0;

	MPU9150_get(i2cptr, A_G_ACCEL_XOUT_H, &rdata);
	d->x            = rdata << 8;
	MPU9150_get(i2cptr, A_G_ACCEL_XOUT_L, &rdata);
	d->x            = (d->x | rdata);
	MPU9150_get(i2cptr, A_G_ACCEL_YOUT_H, &rdata);
	d->y            = rdata << 8;
	MPU9150_get(i2cptr, A_G_ACCEL_YOUT_L, &rdata);
	d->y            = (d->y | rdata);
	MPU9150_get(i2cptr, A_G_ACCEL_ZOUT_H, &rdata);
	d->z            = rdata << 8;
	MPU9150_get(i2cptr, A_G_ACCEL_ZOUT_L, &rdata);
	d->z            = (d->z | rdata);
}

/*! \brief read the gyro x,y,z
 *
 */
void mpu9150_g_read_x_y_z(I2CDriver* i2cptr, MPU9150_gyro_data* d) {

	mpu9150_reg_data rdata = 0;

	MPU9150_get(i2cptr, A_G_GYRO_XOUT_H, &rdata);
	d->x            = rdata << 8;
	MPU9150_get(i2cptr, A_G_GYRO_XOUT_L, &rdata);
	d->x            = (d->x | rdata);
	MPU9150_get(i2cptr, A_G_GYRO_YOUT_H, &rdata);
	d->y            = rdata << 8;
	MPU9150_get(i2cptr, A_G_GYRO_YOUT_L, &rdata);
	d->y            = (d->y | rdata);
	MPU9150_get(i2cptr, A_G_GYRO_ZOUT_H, &rdata);
	d->z            = rdata << 8;
	MPU9150_get(i2cptr, A_G_GYRO_ZOUT_L, &rdata);
	d->z            = (d->z | rdata);
}

/*! \read the magnetometer AK8975C id
 *
 */
void mpu9150_magn_read_id(I2CDriver* i2cptr) {
    mpu9150_reg_data d;
	msg_t status = RDY_OK;
	MPU9150_get(i2cptr, MAGN_DEVICE_ID, &d);
}

//! @}
