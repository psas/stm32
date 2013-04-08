/*! \file MPU9150.h
 *
 * Intended for use with ChibiOS RT
 *
 * InvenSense Inc. Part: MPU-9150
 * Ref: www.invensense.com
 * Based on product specification Revision 4.0
 *
 */

/*! \addtogroup mpu9150
 * @{
 */

#ifndef _MPU9150_H
#define _MPU9150_H


#ifdef __cplusplus
extern "C" {
#endif

#include "ch.h"
#include "hal.h"

#define     DEBUG_MPU9150                         1

#define     MPU9150_MAX_TX_BUFFER                 50
#define     MPU9150_MAX_RX_BUFFER                 50

/*! register 55 INT pin/Bypass */
#define     MPU9150_CLKOUT_EN                     ((uint8_t)(1<<0))
#define     MPU9150_I2C_BYPASS                    ((uint8_t)(1<<1))
#define     MPU9150_FSCYNC_INT_EN                 ((uint8_t)(1<<2))
#define     MPU9150_FSCYNC_INT_LEVEL              ((uint8_t)(1<<3))
#define     MPU9150_INT_RD_CLEAR                  ((uint8_t)(1<<4))
#define     MPU9150_LATCH_INT_EN                  ((uint8_t)(1<<5))
#define     MPU9150_INT_OPEN                      ((uint8_t)(1<<6))
#define     MPU9150_INT_LEVEL                     ((uint8_t)(1<<7))

typedef     uint8_t                               mpu9150_i2c_data;
typedef     uint16_t                              mpu9150_reg_data;
typedef     uint8_t                               mpu9150_reg_addr;

#if DEBUG_MPU9150

/* see hal/include/i2c.h */
typedef struct i2c_error_info {
    const char* err_string;
    int         error_number;
} i2c_error_info;

const char* i2c_errno_str(int32_t err) ;

#endif


/*! \typedef mpu9150_magn_regaddr
 *
 * MPU Magnetometer addresses
 *
 * Note the i2c address of the Magnetometer is 0x0c and is accessed through the auxiliary i2c bus
 * when the mpu9150 is in 'pass-through' mode. See block diagram.
 */
typedef enum {
	MAGN_DEVICE_ID         = 0x00,
	MAGN_INFORMATION       = 0x01,
	MAGN_STATUS_1          = 0x02,
	MAGN_HXL               = 0x03,
	MAGN_HXH               = 0x04,
	MAGN_HYL               = 0x05,
	MAGN_HYH               = 0x06,
	MAGN_HZL               = 0x07,
	MAGN_HZH               = 0x08,
	MAGN_STATUS_2          = 0x09,
	MAGN_CNTL              = 0x0A,
//	MAGN_RSV               = 0x0B, // reserved
	MAGN_ASTC              = 0x0C, //                 self test
	MAGN_TS1               = 0x0D, //                 test 1
	MAGN_TS2               = 0x0E, //                 test 2
	MAGN_I2C_DIS           = 0x0F,
	MAGN_ASAX              = 0x10,
	MAGN_ASAY              = 0x11,
	MAGN_ASAZ              = 0x12
} mpu9150_magn_regaddr;

/*! \typedef mpu9150_a_g_regaddr
 *
 * i2c slave address: 0x68   (see also who_am_i register)
 *
 * MPU Accelerometer and Gyroscope Register addresses
 */
typedef enum {
	// Name         address         default    function
	A_G_SELF_TEST_X        = 0x0D,
    A_G_SELF_TEST_Y        = 0x0E,
    A_G_SELF_TEST_Z        = 0x0F,
    A_G_SELF_TEST_A        = 0x10,
    A_G_SMPLRT_DIV         = 0x19,
    A_G_CONFIG             = 0x1A,
    A_G_GYRO_CONFIG        = 0x1B,
    A_G_ACCEL_CONFIG       = 0x1C,
    A_G_FF_THR             = 0x1D,
    A_G_FF_DUR             = 0x1E,
    A_G_MOT_THR            = 0x1F,
    A_G_MOT_DUR            = 0x20,
    A_G_ZRMOT_THR          = 0x21,
    A_G_ZRMOT_DUR          = 0x22,
    A_G_FIFO_EN            = 0x23,
    A_G_I2C_MST_CTRL       = 0x24,
    A_G_I2C_SLV0_ADDR      = 0x25,
    A_G_I2C_SLV0_REG       = 0x26,
    A_G_I2C_SLV0_CTRL      = 0x27,
    A_G_I2C_SLV1_ADDR      = 0x28,
    A_G_I2C_SLV1_REG       = 0x29,
    A_G_I2C_SLV1_CTRL      = 0x2A,
    A_G_I2C_SLV2_ADDR      = 0x2B,
    A_G_I2C_SLV2_REG       = 0x2C,
    A_G_I2C_SLV2_CTRL      = 0x2D,
    A_G_I2C_SLV3_ADDR      = 0x2E,
    A_G_I2C_SLV3_REG       = 0x2F,
    A_G_I2C_SLV3_CTRL      = 0x30,
    A_G_I2C_SLV4_ADDR      = 0x31,
    A_G_I2C_SLV4_REG       = 0x32,
    A_G_I2C_SLV4_DO        = 0x33,
    A_G_I2C_SLV4_CTRL      = 0x34,
    A_G_I2C_SLV4_DI        = 0x35,
    A_G_I2C_MST_STATUS     = 0x36,
    A_G_INT_PIN_CFG        = 0x37,
    A_G_INT_ENABLE         = 0x38,
    A_G_INT_STATUS         = 0x3A,
    A_G_ACCEL_XOUT_H       = 0x3B,
    A_G_ACCEL_XOUT_L       = 0x3C,
    A_G_ACCEL_YOUT_H       = 0x3D,
    A_G_ACCEL_YOUT_L       = 0x3E,
    A_G_ACCEL_ZOUT_H       = 0x3F,
    A_G_ACCEL_ZOUT_L       = 0x40,
    A_G_TEMP_OUT_H         = 0x41,
    A_G_TEMP_OUT_L         = 0x42,
    A_G_GYRO_XOUT_H        = 0x43,
    A_G_GYRO_XOUT_L        = 0x44,
    A_G_GYRO_YOUT_H        = 0x45,
    A_G_GYRO_YOUT_L        = 0x46,
    A_G_GYRO_ZOUT_H        = 0x47,
    A_G_GYRO_ZOUT_L        = 0x48,
    A_G_EXT_SENS_DATA_00   = 0x49,
    A_G_EXT_SENS_DATA_01   = 0x4A,
    A_G_EXT_SENS_DATA_02   = 0x4B,
    A_G_EXT_SENS_DATA_03   = 0x4C,
    A_G_EXT_SENS_DATA_04   = 0x4D,
    A_G_EXT_SENS_DATA_05   = 0x4E,
    A_G_EXT_SENS_DATA_06   = 0x4F,
    A_G_EXT_SENS_DATA_07   = 0x50,
    A_G_EXT_SENS_DATA_08   = 0x51,
    A_G_EXT_SENS_DATA_09   = 0x52,
    A_G_EXT_SENS_DATA_10   = 0x53,
    A_G_EXT_SENS_DATA_11   = 0x54,
    A_G_EXT_SENS_DATA_12   = 0x55,
    A_G_EXT_SENS_DATA_13   = 0x56,
    A_G_EXT_SENS_DATA_14   = 0x57,
    A_G_EXT_SENS_DATA_15   = 0x58,
    A_G_EXT_SENS_DATA_16   = 0x59,
    A_G_EXT_SENS_DATA_17   = 0x5A,
    A_G_EXT_SENS_DATA_18   = 0x5B,
    A_G_EXT_SENS_DATA_19   = 0x5C,
    A_G_EXT_SENS_DATA_20   = 0x5D,
    A_G_EXT_SENS_DATA_21   = 0x5E,
    A_G_EXT_SENS_DATA_22   = 0x5F,
    A_G_EXT_SENS_DATA_23   = 0x60,
    A_G_MOT_DETECT_STATUS  = 0x61,
    A_G_I2C_SLV0_D0        = 0x63,
    A_G_I2C_SLV1_D0        = 0x64,
    A_G_I2C_SLV2_DO        = 0x65,
    A_G_I2C_SLV3_DO        = 0x66,
    A_G_I2C_MST_DELAY_CTRL = 0x67,
    A_G_SIGNAL_PATH_RESET  = 0x68,
    A_G_MOT_DETECT_CTRL    = 0x69,
    A_G_USER_CTRL          = 0x6A,
    A_G_PWR_MGMT_1         = 0x6B,    // RESET VAL: 0x40
    A_G_PWR_MGMT_2         = 0x6C,
    A_G_FIFO_COUNTH        = 0x72,
    A_G_FIFO_COUNTL        = 0x73,
    A_G_FIFO_R_W           = 0x74,
    A_G_WHO_AM_I           = 0x75     // RESET VAL: 0x68
} mpu9150_a_g_regaddr;

/*! \typedef mpu9150_a_g_burst_data
 * Burst data collection
 *
 */
typedef struct {
	mpu9150_reg_data      mpu_who_ami;
} mpu9150_a_g_burst_data;


/*! \typedef mpu9150_magn_burst_data
 * Burst data collection
 *
 */
typedef struct {
	mpu9150_reg_data      mpu_magn_who_ami;
} mpu9150_magn_burst_data;


/*!
 * Another transaction may begin which would corrupt the tx and rx
 * buffers.
 */
typedef struct {
	uint8_t               current_rx_numbytes;
	uint8_t               current_tx_numbytes;
	mpu9150_a_g_regaddr   a_g_reg;
	mpu9150_magn_regaddr  magn_reg;
	mpu9150_reg_data          tx_cache[MPU9150_MAX_TX_BUFFER];
	mpu9150_reg_data          rx_cache[MPU9150_MAX_RX_BUFFER];
} mpu9150_cache;


/*! \typedef Structure for keeping track of an MPU9150 transaction
 *
 *
 */
typedef struct mpu9150_driver {
	i2cflags_t         i2c_errors;
	I2CDriver*         i2c_instance;                 /*! which stm32f407 I2C instance to use (there are 3)       */
	mpu9150_i2c_data   txbuf[MPU9150_MAX_TX_BUFFER]; /*! Transmit buffer                                         */
	mpu9150_i2c_data   rxbuf[MPU9150_MAX_RX_BUFFER]; /*! Receive buffer                                          */
} MPU9150_Driver;


/*! \typedef mpu9150_config
 *
 * Configuration for the MPU IMU connections
 */
typedef struct {
	/*! \brief The I2C SDA port */
	ioportid_t               i2c_sda_port;
	/*! \brief The I2C SDA pad */
	uint16_t                 i2c_sda_pad;
	/*! \brief The I2C SCL port */
	ioportid_t               i2c_scl_port;
	/*! \brief The I2C SCL pad */
	uint16_t                 i2c_scl_pad;
	/*! \brief The INT port */
	ioportid_t               int_port;
	/*! \brief The INT pad number. */
	uint16_t                 int_pad;
} mpu9150_connect;

extern const I2CConfig                 mpu9150_config;
extern const mpu9150_connect           mpu9150_connections ;
extern       mpu9150_cache             mpu9150_cache_data;

extern       EventSource               mpu9150_int_event;

extern       mpu9150_a_g_burst_data    mpu9150_burst_a_g_data;
extern       mpu9150_magn_burst_data   mpu9150_burst_magn_data;
extern       MPU9150_Driver            mpu9150_driver;

void         mpu9150_int_event_handler(eventid_t id) ;
void         mpu9150_init(I2CDriver* i2c) ;
void         mpu9150_setup(I2CDriver* i2cptr) ;


void         mpu9150_a_g_read_id(I2CDriver* i2cptr) ;
void         mpu9150_magn_read_id(I2CDriver* i2cptr);

/*!
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif


