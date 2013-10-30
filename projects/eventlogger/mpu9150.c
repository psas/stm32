#include "MPU9150.h"
#include "usbdetail.h"

#include "mpu9150.h"

#define MPU9150_DEBUG false


/**
 ** Public Function Definitions
 ******************************/

/*!
 * \brief Initialize mpu9510
 */
void mpu9150_init(I2CDriver* i2cptr) {
	mpu9150_reg_data rdata;

	mpu9150_reset(i2cptr);

	rdata = MPU9150_PM1_X_GYRO_CLOCKREF & (~(MPU9150_PM1_SLEEP));   // make sure device is 'awake'
	mpu9150_write_pm1(i2cptr, rdata);

	rdata = 16;                                          // 2 ms sample period.
	mpu9150_write_gyro_sample_rate_div(i2cptr, rdata);

	rdata = MPU9150_I2C_BYPASS | MPU9150_INT_LEVEL | MPU9150_LATCH_INT_EN;
	mpu9150_write_pin_cfg(i2cptr, rdata);

	rdata = MPU9150_A_HPF_RESET | MPU9150_A_SCALE_pm16g;
	mpu9150_write_accel_config(i2cptr, rdata);

	rdata = MPU9150_G_SCALE_pm2000;
	mpu9150_write_gyro_config(i2cptr, rdata);

	rdata = MPU9150_INT_EN_DATA_RD_EN;
	mpu9150_write_int_enable(i2cptr, rdata);
}


/*
 * This is what actually reads the data from the MPU9150 after we receive an
 * interrupt from it.
 * It is called by the Thread_mpu9150_int_dispatch defined below.
 */
void mpu9150_int_event_handler(eventid_t _) {
#if MPU9150_DEBUG
	static uint16_t count = 0;
  static BaseSequentialStream* chp = (BaseSequentialStream*) &SDU_PSAS;
#endif

	mpu9150_a_read_x_y_z(mpu9150_driver.i2c_instance, &mpu9150_current_read.accel_xyz);
	mpu9150_g_read_x_y_z(mpu9150_driver.i2c_instance, &mpu9150_current_read.gyro_xyz);
	mpu9150_current_read.celsius = mpu9150_a_g_read_temperature(mpu9150_driver.i2c_instance);

	/* clear the interrupt status bits on mpu9150 */
	mpu9150_a_g_read_int_status(mpu9150_driver.i2c_instance);

#if MPU9150_DEBUG
	++count;
	if (count > 500) {
    chprintf(chp, "\r\n*** MPU9150 ***\r\n");
    chprintf(chp, "raw_temp: %d C\r\n", mpu9150_temp_to_dC(mpu9150_current_read.celsius));
    chprintf(chp, "ACCL:  x: %d\ty: %d\tz: %d\r\n", mpu9150_current_read.accel_xyz.x, mpu9150_current_read.accel_xyz.y, mpu9150_current_read.accel_xyz.z);
    chprintf(chp, "GRYO:  x: 0x%x\ty: 0x%x\tz: 0x%x\r\n", mpu9150_current_read.gyro_xyz.x, mpu9150_current_read.gyro_xyz.y, mpu9150_current_read.gyro_xyz.z);
    count = 0;
	}
#endif
}


/*
 * This thread simply waits for the ISR to trigger an mpu9150_int_event, then
 * dispatches to the above mpu9150_int_event_handler function to read the data.
 */
msg_t Thread_mpu9150_int_dispatch(void* _) {
	struct EventListener  evl_mpu9150;
	const  evhandler_t    evhndl_mpu9150[] = { mpu9150_int_event_handler };

	chRegSetThreadName("mpu9150_int");
	mpu9150_init(mpu9150_driver.i2c_instance);
	chEvtRegister(&mpu9150_int_event, &evl_mpu9150, 0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
	}
	return -1;
}


/*
 * Triggers on external interrupt from MPU9150.
 * Simply broadcast an event signifying that new data is ready.
 */
void mpu9150_ISR(EXTDriver *_, expchannel_t __) {
	chSysLockFromIsr();
	chEvtBroadcastI(&mpu9150_int_event);
	chSysUnlockFromIsr();
}
