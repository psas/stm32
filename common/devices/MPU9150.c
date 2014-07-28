/*! \file MPU9150.c
 *
 * API to support reading sensor data from an MPU9150 IMU via I2C.
 *
 * The MPU9150 is an MPU6050 & AKM8975C in the same package with some extra
 * stuff. For this driver the AKM is accessed only in the 9150's master mode.
 */

#include <stddef.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_hal.h"
#include "MPU9150.h"


static int initialized;
static I2CDriver *I2CD;
static const systime_t I2C_TIMEOUT = MS2ST(400);
static EventSource interrupt;

EventSource MPU9150_data_ready;

int MPU9150_Get(uint8_t register_id, uint8_t* data){
	if(initialized == false)
		return -1;

	uint8_t tx[] = {register_id};
	uint8_t rx[1];
	i2cflags_t errors;
	i2cAcquireBus(I2CD);
	msg_t status = i2cMasterTransmitTimeout(I2CD,  MPU9150_a_g_ADDR, tx, sizeof(tx), rx, sizeof(rx), I2C_TIMEOUT);
	switch(status){
	case RDY_OK:
		i2cReleaseBus(I2CD);
		break;
	case RDY_RESET:
		errors = i2cGetErrors(I2CD);
		i2cReleaseBus(I2CD);
		return errors;
	case RDY_TIMEOUT:
		i2cReleaseBus(I2CD);
		return RDY_TIMEOUT;
	default:
		i2cReleaseBus(I2CD);
	}

	*data = rx[0];
	return RDY_OK;
}
int MPU9150_Set(uint8_t register_id, uint8_t data){
	if(initialized == false)
		return -1;

	uint8_t tx[] = {register_id, data};
	i2cflags_t errors;
	i2cAcquireBus(I2CD);
	msg_t status = i2cMasterTransmitTimeout(I2CD, MPU9150_a_g_ADDR, tx, sizeof(tx), NULL, 0, I2C_TIMEOUT);
	switch(status){
	case RDY_OK:
		i2cReleaseBus(I2CD);
		break;
	case RDY_RESET:
		errors = i2cGetErrors(I2CD);
		i2cReleaseBus(I2CD);
		return errors;
	case RDY_TIMEOUT:
		i2cReleaseBus(I2CD);
		return RDY_TIMEOUT;
	default:
		i2cReleaseBus(I2CD);
	}

	return RDY_OK;
}


static void on_interrupt(EXTDriver *extp UNUSED, expchannel_t channel UNUSED){
	chSysLockFromIsr();
	chEvtBroadcastI(&interrupt);
	chSysUnlockFromIsr();
}

static void get_all_sensors(eventid_t u){
	i2cMasterTransmitTimeout();
	chEvtBroadcast(&MPU9150_data_ready);
}

static WORKING_AREA(wa_read, 128);
static msg_t read_thd(void * arg UNUSED){
	chRegSetThreadName("MPU9150");

	struct EventListener listener;
	static const evhandler_t handlers[] = {
		get_all_sensors
	};
	chEvtRegister(&interrupt, &listener, 0);

	while (TRUE) {
		chEvtDispatch(handlers, chEvtWaitOne(ALL_EVENTS));
	}
	return -1;
}


/*! \brief Initialize MPU9150 driver
 *
 */
void MPU9150_init(MPU9150Config  *conf) {
//TODO: If I2C is active, check if config is correct

	I2CD = conf->I2CD;

	static const I2CConfig i2cfg = {
		OPMODE_I2C,
		400000,
		FAST_DUTY_CYCLE_2,
	};
	i2cUtilsStart(I2CD, &i2cfg, &(conf->pins));
	chEvtInit(&MPU9150_data_ready);
	chEvtinit(&interrupt);

	extAddCallback(conf->interrupt, EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, on_interrupt);
	extUtilsStart();

	chThdCreateStatic(wa_read, sizeof(wa_read), NORMALPRIO, read_thd, NULL);

	initialized = true;
}

void mpu9150_reset(void) {
	/*! Turn on power */
	MPU9150_set(I2CD, A_G_PWR_MGMT_1, MPU9150_PM1_RESET);
	chThdSleepMilliseconds(200);  // wait for device reset

	MPU9150_set(I2CD, A_G_SIGNAL_PATH_RESET, 0b111);
	chThdSleepMilliseconds(200);  // wait for signal path reset
}

void MPU9150_set_pm1(mpu9150_reg_data d) {
	/*! Turn on power */
	MPU9150_set(A_G_PWR_MGMT_1, &d);
}

void MPU9150_set_pin_cfg(mpu9150_reg_data d) {
	MPU9150_set(A_G_INT_PIN_CFG, &d);
}

void MPU9150_set_int_enable(mpu9150_reg_data d) {
	MPU9150_set(A_G_INT_ENABLE, &d);
}

void MPU9150_set_accel_config(mpu9150_reg_data d) {
	MPU9150_set(A_G_ACCEL_CONFIG, &d);
}

void MPU9150_set_gyro_sample_rate_div(mpu9150_reg_data d) {
	MPU9150_set(A_G_SMPLRT_DIV, &d);
}

void MPU9150_set_gyro_config(mpu9150_reg_data d) {
	MPU9150_set(A_G_GYRO_CONFIG, &d);
}

void MPU9150_set_fifo_en(mpu9150_reg_data d) {
	MPU9150_set(A_G_FIFO_EN, &d);
}

/*! \brief read the accel-gyro id
 *
 */
void mpu9150_a_g_read_id(void) {
	mpu9150_reg_data d;
	MPU9150_get(A_G_WHO_AM_I, &d);
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
int16_t mpu9150_a_g_read_temperature(void) {
	mpu9150_reg_data d;

	int16_t		 raw_temp = 0;

	MPU9150_get(A_G_TEMP_OUT_H, &d);
	raw_temp = mpu9150_driver.rxbuf[0] << 8;

	MPU9150_get(A_G_TEMP_OUT_L, &d);
	raw_temp = raw_temp | mpu9150_driver.rxbuf[0];

	return raw_temp;
}

/*! \brief read the interrupt status register
 *
 * Clears interrupt bits
 */
void mpu9150_a_g_read_int_status(void) {
	mpu9150_reg_data d;
	MPU9150_get(A_G_INT_STATUS, &d);
}

/*! \brief read the accel-gyro fifo count
 *
 */
uint16_t mpu9150_a_g_fifo_cnt(void) {
	mpu9150_reg_data d;
	uint16_t fifo_count = 0;

	MPU9150_get(A_G_FIFO_COUNTH, &d);
	fifo_count = mpu9150_driver.rxbuf[0] << 8;

	MPU9150_get(A_G_FIFO_COUNTL, &d);
	fifo_count = fifo_count | (mpu9150_driver.rxbuf[0] << 8);

	return fifo_count;
}

/*! \brief read the accel x,y,z
 *
 */
void mpu9150_a_read_x_y_z(MPU9150_accel_data* d) {
	mpu9150_reg_data rdata = 0;

	MPU9150_get(A_G_ACCEL_XOUT_H, &rdata);
	d->x = rdata << 8;
	MPU9150_get(A_G_ACCEL_XOUT_L, &rdata);
	d->x = (d->x | rdata);
	MPU9150_get(A_G_ACCEL_YOUT_H, &rdata);
	d->y = rdata << 8;
	MPU9150_get(A_G_ACCEL_YOUT_L, &rdata);
	d->y = (d->y | rdata);
	MPU9150_get(A_G_ACCEL_ZOUT_H, &rdata);
	d->z = rdata << 8;
	MPU9150_get(A_G_ACCEL_ZOUT_L, &rdata);
	d->z = (d->z | rdata);
}

/*! \brief read the gyro x,y,z
 *
 */
void mpu9150_g_read_x_y_z(MPU9150_gyro_data* d) {

	mpu9150_reg_data rdata = 0;

	MPU9150_get(A_G_GYRO_XOUT_H, &rdata);
	d->x = rdata << 8;
	MPU9150_get(A_G_GYRO_XOUT_L, &rdata);
	d->x= (d->x | rdata);
	MPU9150_get(A_G_GYRO_YOUT_H, &rdata);
	d->y= rdata << 8;
	MPU9150_get(A_G_GYRO_YOUT_L, &rdata);
	d->y = (d->y | rdata);
	MPU9150_get(A_G_GYRO_ZOUT_H, &rdata);
	d->z = rdata << 8;
	MPU9150_get(A_G_GYRO_ZOUT_L, &rdata);
	d->z = (d->z | rdata);
}

/*! \read the magnetometer AK8975C id
 *
 */
void mpu9150_magn_read_id(void) {
	mpu9150_reg_data d;
	MPU9150_get(MAGN_DEVICE_ID, &d);
}

//! @}
