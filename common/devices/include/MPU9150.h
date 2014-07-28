/*! \file MPU9150.h
 *
 * Intended for use with ChibiOS RT
 *
 * InvenSense Inc. Part: MPU-9150
 * Ref: www.invensense.com
 * Based on product specification Revision 4.0
 *
 */

#ifndef _MPU9150_H
#define _MPU9150_H

#include "ch.h"
#include "hal.h"

#include "utils_hal.h"

/*! register 55 INT pin/Bypass */
#define     MPU9150_CLKOUT_EN                     ((mpu9150_reg_data)(1<<0))
#define     MPU9150_I2C_BYPASS                    ((mpu9150_reg_data)(1<<1))
#define     MPU9150_FSCYNC_INT_EN                 ((mpu9150_reg_data)(1<<2))
#define     MPU9150_FSCYNC_INT_LEVEL              ((mpu9150_reg_data)(1<<3))
#define     MPU9150_INT_RD_CLEAR                  ((mpu9150_reg_data)(1<<4))
#define     MPU9150_LATCH_INT_EN                  ((mpu9150_reg_data)(1<<5))
#define     MPU9150_INT_OPEN                      ((mpu9150_reg_data)(1<<6))
#define     MPU9150_INT_LEVEL                     ((mpu9150_reg_data)(1<<7))

/*! register 107 Power management 1 */
#define     MPU9150_PM1_X_GYRO_CLOCKREF           ((mpu9150_reg_data)(1<<0))
#define     MPU9150_PM1_SLEEP                     ((mpu9150_reg_data)(1<<6))
#define     MPU9150_PM1_RESET                     ((mpu9150_reg_data)(1<<7))
#define     MPU9150_INT_EN_DATA_RD_EN             ((mpu9150_reg_data)(1<<0))


#define MPU9150_a_g_ADDR 0x68    // See page 8 , MPU9150 Register Map and Descriptions r4.0
#define MPU9150_magn_ADDR 0x0C    // See page 28, MPU9150 Product Specification r4.0

/*! \typedef mpu9150_magn_regaddr
 * MPU Magnetometer addresses
 *
 * Note the i2c address of the Magnetometer is 0x0c and is accessed through the auxiliary i2c bus
 * when the mpu9150 is in 'pass-through' mode. See block diagram.
 */
typedef enum {
	MAGN_DEVICE_ID         = 0x00,//!< MAGN_DEVICE_ID
	MAGN_INFORMATION       = 0x01,//!< MAGN_INFORMATION
	MAGN_STATUS_1          = 0x02,//!< MAGN_STATUS_1
	MAGN_HXL               = 0x03,//!< MAGN_HXL
	MAGN_HXH               = 0x04,//!< MAGN_HXH
	MAGN_HYL               = 0x05,//!< MAGN_HYL
	MAGN_HYH               = 0x06,//!< MAGN_HYH
	MAGN_HZL               = 0x07,//!< MAGN_HZL
	MAGN_HZH               = 0x08,//!< MAGN_HZH
	MAGN_STATUS_2          = 0x09,//!< MAGN_STATUS_2
	MAGN_CNTL              = 0x0A,//!< MAGN_CNTL
	MAGN_ASTC              = 0x0C,//!< MAGN_ASTC
	MAGN_TS1               = 0x0D,//!< MAGN_TS1
	MAGN_TS2               = 0x0E,//!< MAGN_TS2
	MAGN_I2C_DIS           = 0x0F,//!< MAGN_I2C_DIS
	MAGN_ASAX              = 0x10,//!< MAGN_ASAX
	MAGN_ASAY              = 0x11,//!< MAGN_ASAY
	MAGN_ASAZ              = 0x12 //!< MAGN_ASAZ
} mpu9150_magn_regaddr;


/*! \typedef mpu9150_a_g_regaddr
 *
 * i2c slave address: 0x68   (see also who_am_i register)
 *
 * MPU Accelerometer and Gyroscope Register addresses
 */
typedef enum {
	A_G_SELF_TEST_X        = 0x0D,   //!< A_G_SELF_TEST_X
	A_G_SELF_TEST_Y        = 0x0E,//!< A_G_SELF_TEST_Y
	A_G_SELF_TEST_Z        = 0x0F,//!< A_G_SELF_TEST_Z
	A_G_SELF_TEST_A        = 0x10,//!< A_G_SELF_TEST_A
	A_G_SMPLRT_DIV         = 0x19,//!< A_G_SMPLRT_DIV
	A_G_CONFIG             = 0x1A,//!< A_G_CONFIG
	A_G_GYRO_CONFIG        = 0x1B,//!< A_G_GYRO_CONFIG
	A_G_ACCEL_CONFIG       = 0x1C,//!< A_G_ACCEL_CONFIG
	A_G_FF_THR             = 0x1D,//!< A_G_FF_THR
	A_G_FF_DUR             = 0x1E,//!< A_G_FF_DUR
	A_G_MOT_THR            = 0x1F,//!< A_G_MOT_THR
	A_G_MOT_DUR            = 0x20,//!< A_G_MOT_DUR
	A_G_ZRMOT_THR          = 0x21,//!< A_G_ZRMOT_THR
	A_G_ZRMOT_DUR          = 0x22,//!< A_G_ZRMOT_DUR
	A_G_FIFO_EN            = 0x23,//!< A_G_FIFO_EN
	A_G_I2C_MST_CTRL       = 0x24,//!< A_G_I2C_MST_CTRL
	A_G_I2C_SLV0_ADDR      = 0x25,//!< A_G_I2C_SLV0_ADDR
	A_G_I2C_SLV0_REG       = 0x26,//!< A_G_I2C_SLV0_REG
	A_G_I2C_SLV0_CTRL      = 0x27,//!< A_G_I2C_SLV0_CTRL
	A_G_I2C_SLV1_ADDR      = 0x28,//!< A_G_I2C_SLV1_ADDR
	A_G_I2C_SLV1_REG       = 0x29,//!< A_G_I2C_SLV1_REG
	A_G_I2C_SLV1_CTRL      = 0x2A,//!< A_G_I2C_SLV1_CTRL
	A_G_I2C_SLV2_ADDR      = 0x2B,//!< A_G_I2C_SLV2_ADDR
	A_G_I2C_SLV2_REG       = 0x2C,//!< A_G_I2C_SLV2_REG
	A_G_I2C_SLV2_CTRL      = 0x2D,//!< A_G_I2C_SLV2_CTRL
	A_G_I2C_SLV3_ADDR      = 0x2E,//!< A_G_I2C_SLV3_ADDR
	A_G_I2C_SLV3_REG       = 0x2F,//!< A_G_I2C_SLV3_REG
	A_G_I2C_SLV3_CTRL      = 0x30,//!< A_G_I2C_SLV3_CTRL
	A_G_I2C_SLV4_ADDR      = 0x31,//!< A_G_I2C_SLV4_ADDR
	A_G_I2C_SLV4_REG       = 0x32,//!< A_G_I2C_SLV4_REG
	A_G_I2C_SLV4_DO        = 0x33,//!< A_G_I2C_SLV4_DO
	A_G_I2C_SLV4_CTRL      = 0x34,//!< A_G_I2C_SLV4_CTRL
	A_G_I2C_SLV4_DI        = 0x35,//!< A_G_I2C_SLV4_DI
	A_G_I2C_MST_STATUS     = 0x36,//!< A_G_I2C_MST_STATUS
	A_G_INT_PIN_CFG        = 0x37,//!< A_G_INT_PIN_CFG
	A_G_INT_ENABLE         = 0x38,//!< A_G_INT_ENABLE
	A_G_INT_STATUS         = 0x3A,//!< A_G_INT_STATUS
	A_G_ACCEL_XOUT_H       = 0x3B,//!< A_G_ACCEL_XOUT_H
	A_G_ACCEL_XOUT_L       = 0x3C,//!< A_G_ACCEL_XOUT_L
	A_G_ACCEL_YOUT_H       = 0x3D,//!< A_G_ACCEL_YOUT_H
	A_G_ACCEL_YOUT_L       = 0x3E,//!< A_G_ACCEL_YOUT_L
	A_G_ACCEL_ZOUT_H       = 0x3F,//!< A_G_ACCEL_ZOUT_H
	A_G_ACCEL_ZOUT_L       = 0x40,//!< A_G_ACCEL_ZOUT_L
	A_G_TEMP_OUT_H         = 0x41,//!< A_G_TEMP_OUT_H
	A_G_TEMP_OUT_L         = 0x42,//!< A_G_TEMP_OUT_L
	A_G_GYRO_XOUT_H        = 0x43,//!< A_G_GYRO_XOUT_H
	A_G_GYRO_XOUT_L        = 0x44,//!< A_G_GYRO_XOUT_L
	A_G_GYRO_YOUT_H        = 0x45,//!< A_G_GYRO_YOUT_H
	A_G_GYRO_YOUT_L        = 0x46,//!< A_G_GYRO_YOUT_L
	A_G_GYRO_ZOUT_H        = 0x47,//!< A_G_GYRO_ZOUT_H
	A_G_GYRO_ZOUT_L        = 0x48,//!< A_G_GYRO_ZOUT_L
	A_G_EXT_SENS_DATA_00   = 0x49,//!< A_G_EXT_SENS_DATA_00
	A_G_EXT_SENS_DATA_01   = 0x4A,//!< A_G_EXT_SENS_DATA_01
	A_G_EXT_SENS_DATA_02   = 0x4B,//!< A_G_EXT_SENS_DATA_02
	A_G_EXT_SENS_DATA_03   = 0x4C,//!< A_G_EXT_SENS_DATA_03
	A_G_EXT_SENS_DATA_04   = 0x4D,//!< A_G_EXT_SENS_DATA_04
	A_G_EXT_SENS_DATA_05   = 0x4E,//!< A_G_EXT_SENS_DATA_05
	A_G_EXT_SENS_DATA_06   = 0x4F,//!< A_G_EXT_SENS_DATA_06
	A_G_EXT_SENS_DATA_07   = 0x50,//!< A_G_EXT_SENS_DATA_07
	A_G_EXT_SENS_DATA_08   = 0x51,//!< A_G_EXT_SENS_DATA_08
	A_G_EXT_SENS_DATA_09   = 0x52,//!< A_G_EXT_SENS_DATA_09
	A_G_EXT_SENS_DATA_10   = 0x53,//!< A_G_EXT_SENS_DATA_10
	A_G_EXT_SENS_DATA_11   = 0x54,//!< A_G_EXT_SENS_DATA_11
	A_G_EXT_SENS_DATA_12   = 0x55,//!< A_G_EXT_SENS_DATA_12
	A_G_EXT_SENS_DATA_13   = 0x56,//!< A_G_EXT_SENS_DATA_13
	A_G_EXT_SENS_DATA_14   = 0x57,//!< A_G_EXT_SENS_DATA_14
	A_G_EXT_SENS_DATA_15   = 0x58,//!< A_G_EXT_SENS_DATA_15
	A_G_EXT_SENS_DATA_16   = 0x59,//!< A_G_EXT_SENS_DATA_16
	A_G_EXT_SENS_DATA_17   = 0x5A,//!< A_G_EXT_SENS_DATA_17
	A_G_EXT_SENS_DATA_18   = 0x5B,//!< A_G_EXT_SENS_DATA_18
	A_G_EXT_SENS_DATA_19   = 0x5C,//!< A_G_EXT_SENS_DATA_19
	A_G_EXT_SENS_DATA_20   = 0x5D,//!< A_G_EXT_SENS_DATA_20
	A_G_EXT_SENS_DATA_21   = 0x5E,//!< A_G_EXT_SENS_DATA_21
	A_G_EXT_SENS_DATA_22   = 0x5F,//!< A_G_EXT_SENS_DATA_22
	A_G_EXT_SENS_DATA_23   = 0x60,//!< A_G_EXT_SENS_DATA_23
	A_G_MOT_DETECT_STATUS  = 0x61,//!< A_G_MOT_DETECT_STATUS
	A_G_I2C_SLV0_D0        = 0x63,//!< A_G_I2C_SLV0_D0
	A_G_I2C_SLV1_D0        = 0x64,//!< A_G_I2C_SLV1_D0
	A_G_I2C_SLV2_DO        = 0x65,//!< A_G_I2C_SLV2_DO
	A_G_I2C_SLV3_DO        = 0x66,//!< A_G_I2C_SLV3_DO
	A_G_I2C_MST_DELAY_CTRL = 0x67,//!< A_G_I2C_MST_DELAY_CTRL
	A_G_SIGNAL_PATH_RESET  = 0x68,//!< A_G_SIGNAL_PATH_RESET
	A_G_MOT_DETECT_CTRL    = 0x69,//!< A_G_MOT_DETECT_CTRL
	A_G_USER_CTRL          = 0x6A,//!< A_G_USER_CTRL
	A_G_PWR_MGMT_1         = 0x6B,//!< A_G_PWR_MGMT_1
	A_G_PWR_MGMT_2         = 0x6C,//!< A_G_PWR_MGMT_2
	A_G_FIFO_COUNTH        = 0x72,//!< A_G_FIFO_COUNTH
	A_G_FIFO_COUNTL        = 0x73,//!< A_G_FIFO_COUNTL
	A_G_FIFO_R_W           = 0x74,//!< A_G_FIFO_R_W
	A_G_WHO_AM_I           = 0x75 //!< A_G_WHO_AM_I
} mpu9150_a_g_regaddr;

/*! \typedef Accel High Pass Filter
 * See Accel Config register ACCEL_HPF[2:0]
 */
typedef enum mpu9150_accel_hpf {
	MPU9150_A_HPF_RESET  = 0b000,//!< MPU9150_A_HPF_RESET
	MPU9150_A_HPF_5HZ    = 0b001,//!< MPU9150_A_HPF_5HZ
	MPU9150_A_HPF_2p5HZ  = 0b010,//!< MPU9150_A_HPF_2p5HZ
	MPU9150_A_HPF_1p25HZ = 0b011,//!< MPU9150_A_HPF_1p25HZ
	MPU9150_A_HPF_0p63HZ = 0b100,//!< MPU9150_A_HPF_0p63HZ
	MPU9150_A_HPF_HOLD   = 0b111 //!< MPU9150_A_HPF_HOLD
} mpu9150_accel_hpf;

/*! \typedef Accel Full Scale
 * See Accel Config AFS_SEL[1:0]
 */
typedef enum mpu9150_accel_scale {
	MPU9150_A_SCALE_pm2g   = (0b00 << 3),//!< MPU9150_A_SCALE_pm2g
	MPU9150_A_SCALE_pm4g   = (0b01 << 3),//!< MPU9150_A_SCALE_pm4g
	MPU9150_A_SCALE_pm8g   = (0b10 << 3),//!< MPU9150_A_SCALE_pm8g
	MPU9150_A_SCALE_pm16g  = (0b11 << 3) //!< MPU9150_A_SCALE_pm16g
} mpu9150_accel_scale;


/*! \typedef Gyro Full Scale
 *
 */
typedef enum mpu9150_gyro_scale {
	MPU9150_G_SCALE_pm250   = (0b00 << 3),//!< MPU9150_G_SCALE_pm250
	MPU9150_G_SCALE_pm500   = (0b01 << 3),//!< MPU9150_G_SCALE_pm500
	MPU9150_G_SCALE_pm1000  = (0b10 << 3),//!< MPU9150_G_SCALE_pm1000
	MPU9150_G_SCALE_pm2000  = (0b11 << 3) //!< MPU9150_G_SCALE_pm2000
} mpu9150_gyro_scale;

/*! \typedef Structure for accelerometer data
 *
 *
 */
struct MPU9150_accel_data {
	uint16_t x;
	uint16_t y;
	uint16_t z;
}  __attribute__((packed)) ;
typedef struct MPU9150_accel_data MPU9150_accel_data;

/*! \typedef Structure for gyroscope data
 *
 *
 */
struct MPU9150_gyro_data {
	uint16_t x;
	uint16_t y;
	uint16_t z;
}  __attribute__((packed)) ;
typedef struct MPU9150_gyro_data MPU9150_gyro_data;

/*! \typedef Read Data from mpu9150
 *
 */
struct MPU9150_read_data {
	MPU9150_gyro_data     gyro_xyz;
	MPU9150_accel_data    accel_xyz;
	int16_t               celsius;
}  __attribute__((packed)) ;

typedef struct MPU9150_read_data MPU9150_read_data;

/*! \typedef mpu9150_config
 *
 * Configuration for the MPU IMU connections
 */
typedef struct {
	struct pin sda;
	struct pin scl;
	struct pin interrupt;
	I2CDriver * I2CD;
} MPU9150Config;


extern EventSource mpu9150_data_event;

void         mpu9150_start(I2CDriver* i2c) ;
void         mpu9150_reset(I2CDriver* i2cptr) ;

void         mpu9150_write_gyro_config(I2CDriver* i2cptr, mpu9150_reg_data d) ;
void         mpu9150_write_accel_config(I2CDriver* i2cptr, mpu9150_reg_data d) ;
void         mpu9150_a_g_read_id(I2CDriver* i2cptr) ;
void         mpu9150_magn_read_id(I2CDriver* i2cptr);
void         mpu9150_write_pm1(I2CDriver* i2cptr, mpu9150_reg_data d) ;
void         mpu9150_write_pin_cfg(I2CDriver* i2cptr, mpu9150_reg_data d) ;
void         mpu9150_write_int_enable(I2CDriver* i2cptr, mpu9150_reg_data d) ;
void         mpu9150_a_g_read_int_status(I2CDriver* i2cptr) ;
void         mpu9150_a_read_x_y_z(I2CDriver* i2cptr, MPU9150_accel_data* d) ;
void         mpu9150_g_read_x_y_z(I2CDriver* i2cptr, MPU9150_gyro_data* d) ;
void         mpu9150_write_gyro_sample_rate_div(I2CDriver* i2cptr, mpu9150_reg_data d) ;
int16_t      mpu9150_temp_to_dC(int16_t raw_temp) ;
int16_t      mpu9150_a_g_read_temperature(I2CDriver* i2cptr) ;

/*!
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif


