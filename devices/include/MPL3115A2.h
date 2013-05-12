/*! \file MPL3115A2.h
 *
 * Intended for use with ChibiOS RT
 *
 * InvenSense Inc. Part: MPU-9150
 * Ref: www.invensense.com
 * Based on product specification Revision 4.0
 *
 */

/*! \addtogroup mpl3115a2
 * @{
 */

#ifndef _MPL3115A2_H
#define _MPL3115A2_H


#ifdef __cplusplus
extern "C" {
#endif

#include "ch.h"
#include "hal.h"

typedef     uint8_t                               mpl3115a2_i2c_data;
typedef     uint8_t                               mpl3115a2_reg_data;
typedef     uint8_t                               mpl3115a2_reg_addr;

#define     DEBUG_MPL3115A2                         1

#define     MPL3115A2_MAX_TX_BUFFER                 50
#define     MPL3115A2_MAX_RX_BUFFER                 50

/*! register 55 INT pin/Bypass */
#define     MPL3115A2_CLKOUT_EN                     ((mpl3115a2_reg_data)(1<<0))
#define     MPL3115A2_I2C_BYPASS                    ((mpl3115a2_reg_data)(1<<1))
#define     MPL3115A2_FSCYNC_INT_EN                 ((mpl3115a2_reg_data)(1<<2))
#define     MPL3115A2_FSCYNC_INT_LEVEL              ((mpl3115a2_reg_data)(1<<3))
#define     MPL3115A2_INT_RD_CLEAR                  ((mpl3115a2_reg_data)(1<<4))
#define     MPL3115A2_LATCH_INT_EN                  ((mpl3115a2_reg_data)(1<<5))
#define     MPL3115A2_INT_OPEN                      ((mpl3115a2_reg_data)(1<<6))
#define     MPL3115A2_INT_LEVEL                     ((mpl3115a2_reg_data)(1<<7))

/*! register 107 Power management 1 */
#define     MPL3115A2_X_GYRO_CLOCKREF               ((mpl3115a2_reg_data)(1<<0))

#if DEBUG_MPL3115A2

/* see hal/include/i2c.h */
typedef struct i2c_error_info {
    const char* err_string;
    int         error_number;
} i2c_error_info;

const char* i2c_errno_str(int32_t err) ;

#endif


/*! \typedef mpl3115a2_magn_regaddr
 *
 * MPU Magnetometer addresses
 *
 * Note the i2c address of the Magnetometer is 0x0c and is accessed through the auxiliary i2c bus
 * when the mpl3115a2 is in 'pass-through' mode. See block diagram.
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
} mpl3115a2_magn_regaddr;
/*! \typedef mpl3115a2regaddr
 *
 * i2c slave address: 0x60   (see also who_am_i register)
 *
 * mpl3115a2 temp/pressure sensor
 */
typedef enum  {
	STATUS 						= 0x00,
	OUT_P_MSB					= 0x01,
	OUT_P_CSB					= 0x02,
	OUT_P_LSB					= 0x03,
	OUT_T_MSB					= 0x04,
	OUT_T_LSB					= 0x05,
	DR_STATUS					= 0x06,
	OUT_P_DELTA_MSB			= 0x07,
	OUT_P_DELTA_CSB			= 0x08,
	OUT_P_DELTA_LSB			= 0x09,
	OUT_T_DELTA_MSB			= 0x0A,
	OUT_T_DELTA_LSB			= 0x0B,
	WHO_AM_I						= 0x0C, //this is the MPL3115A2's address
	F_STATUS						= 0x0D,
	F_DATA						= 0x0E,
	F_SETUP						= 0x0F,
	TIME_DLY						= 0x10,
	SYSMOD						= 0x11,
	INT_SOURCE					= 0x12,
	PT_DATA_CFG					= 0x13,
	BAR_IN_MSB					= 0x14,
	BAR_IN_LSB					= 0x15,
	P_TGT_MSB					= 0x16,
	P_TGT_LSB					= 0x17,
	T_TGT							= 0x18,
	P_WND_MSB					= 0x19,
	P_WND_LSB					= 0x1A,
	T_WND							= 0x1B,
	P_MIN_MSB					= 0x1C,
	P_MIN_CSB					= 0x1D,
	P_MIN_LSB					= 0x1E,
	T_MIN_MSB					= 0x1F,
	T_MIN_LSB					= 0x20,
	P_MAX_MSB					= 0x21,
	P_MAX_CSB					= 0x22,
	P_MAX_LSB					= 0x23,
	T_MAX_MSB					= 0x24,
	T_MAX_LSB					= 0x25,
	CTRL_REG1					= 0x26,
	CTRL_REG2					= 0x27,
	CTRL_REG3					= 0x28,
	CTRL_REG4					= 0x29,
	CTRL_REG5					= 0x2A,
	OFF_P							= 0x2B,
	OFF_T							= 0x2C,
	OFF_H							= 0x2D,
	MPL_SLAVE_ADDR				= 0x60
} mpl3115a2_regaddr;

/*! \typedef mpl3115a2_a_g_burst_data
 * Burst data collection
 *
 */
typedef struct {
	mpl3115a2_reg_data      mpu_who_ami;
} mpl3115a2_a_g_burst_data;


/*! \typedef mpl3115a2_magn_burst_data
 * Burst data collection
 *
 */
typedef struct {
	mpl3115a2_reg_data      mpu_magn_who_ami;
} mpl3115a2_magn_burst_data;


/*!
 * Another transaction may begin which would corrupt the tx and rx
 * buffers.
 */
typedef struct {
	uint8_t               current_rx_numbytes;
	uint8_t               current_tx_numbytes;
	mpl3115a2_regaddr   a_g_reg;
	mpl3115a2_magn_regaddr  magn_reg;
	mpl3115a2_reg_data          tx_cache[MPL3115A2_MAX_TX_BUFFER];
	mpl3115a2_reg_data          rx_cache[MPL3115A2_MAX_RX_BUFFER];
} mpl3115a2_cache;


/*! \typedef Structure for keeping track of an MPL3115A2 transaction
 *
 *
 */
typedef struct mpl3115a2_driver {
	i2cflags_t         i2c_errors;
	I2CDriver*         i2c_instance;                 /*! which stm32f407 I2C instance to use (there are 3)       */
	mpl3115a2_i2c_data   txbuf[MPL3115A2_MAX_TX_BUFFER]; /*! Transmit buffer                                         */
	mpl3115a2_i2c_data   rxbuf[MPL3115A2_MAX_RX_BUFFER]; /*! Receive buffer                                          */
	float						temp;
} MPL3115A2_Driver;


/*! \typedef mpl3115a2_config
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
} mpl3115a2_connect;

extern const I2CConfig                 mpl3115a2_config;
extern const mpl3115a2_connect           mpl3115a2_connections ;
extern       mpl3115a2_cache             mpl3115a2_cache_data;

extern       EventSource               mpl3115a2_int_event;

extern       mpl3115a2_a_g_burst_data    mpl3115a2_burst_a_g_data;
extern       mpl3115a2_magn_burst_data   mpl3115a2_burst_magn_data;
extern       MPL3115A2_Driver            mpl3115a2_driver;

void         mpl3115a2_int_event_handler(eventid_t id) ;
void         mpl3115a2_start(I2CDriver* i2c) ;
/*
void         mpl3115a2_init(I2CDriver* i2c) ;

void         mpl3115a2_setup(I2CDriver* i2cptr) ;
void         mpl3115a2_test(I2CDriver* i2cptr) ;


void         mpl3115a2_a_g_read_id(I2CDriver* i2cptr) ;
void         mpl3115a2_magn_read_id(I2CDriver* i2cptr);
void         mpl3115a2_write_pm1(I2CDriver* i2cptr, mpl3115a2_reg_data d) ;
*/
msg_t mpl3115a2_init(I2CDriver* i2cptr);
float mpl3115a2_get_altitude (I2CDriver* i2cptr);
void mpl3115a2_get_temperature (I2CDriver* i2cptr);

/*!
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif


