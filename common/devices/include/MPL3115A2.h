/*! \file MPL3115A2.h
 *
 * Freescale Semiconductor Pressure sensor
 *
 * Intended for use with ChibiOS RT
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

typedef     uint8_t                                 mpl3115a2_i2c_data;
typedef     uint32_t                                mpl3115a2_pressure_data;
typedef     uint32_t                                mpl3115a2_temperature_data;

#if !defined(DEBUG_MPL3115A2) || defined(__DOXYGEN__)
	#define 	DEBUG_MPL3115A2                  0
#endif

#define     MPL3115A2_MAX_TX_BUFFER                 50
#define     MPL3115A2_MAX_RX_BUFFER                 50
#define     MPL3115A2_CTL1_ALT_BIT                   7
#define     MPL3115A2_CTL1_SBYB_BIT                  0
#define     MPL3115A2_CTL1_OST_BIT                   1
#define     MPL3115A2_CTL1_RST_BIT                   2
#define     MPL3115A2_CTL1_OS_BITS                   3
#define     MPL3115A2_CTRL4_DRDY_INT_BIT             7
#define     MPL3115A2_CTRL5_INT_CFG_DRDY             7

#if DEBUG_MPL3115A2
/* see hal/include/i2c.h */
typedef struct i2c_error_info {
    const char* err_string;
    int         error_number;
} i2c_error_info;

const char* i2c_errno_str(int32_t err) ;
#endif

/*! \typedef mpl3115a2 regaddr
 *
 * i2c slave address: 0x60
 *
 * mpl3115a2 temp/pressure sensor
 */
typedef enum  {
	MPL_STATUS 						= 0x00,
	MPL_OUT_P_MSB					= 0x01,
	MPL_OUT_P_CSB					= 0x02,
	MPL_OUT_P_LSB					= 0x03,
	MPL_OUT_T_MSB					= 0x04,
	MPL_OUT_T_LSB					= 0x05,
	MPL_DR_STATUS					= 0x06,
	MPL_OUT_P_DELTA_MSB			    = 0x07,
	MPL_OUT_P_DELTA_CSB			    = 0x08,
	MPL_OUT_P_DELTA_LSB			    = 0x09,
	MPL_OUT_T_DELTA_MSB			    = 0x0A,
	MPL_OUT_T_DELTA_LSB			    = 0x0B,
	MPL_WHO_AM_I				    = 0x0C,
	MPL_F_STATUS				    = 0x0D,
	MPL_F_DATA						= 0x0E,
	MPL_F_SETUP						= 0x0F,
	MPL_TIME_DLY				    = 0x10,
	MPL_SYSMOD						= 0x11,
	MPL_INT_SOURCE					= 0x12,
	MPL_PT_DATA_CFG					= 0x13,
	MPL_BAR_IN_MSB					= 0x14,
	MPL_BAR_IN_LSB					= 0x15,
	MPL_P_TGT_MSB					= 0x16,
	MPL_P_TGT_LSB					= 0x17,
	MPL_T_TGT						= 0x18,
	MPL_P_WND_MSB					= 0x19,
	MPL_P_WND_LSB					= 0x1A,
	MPL_T_WND						= 0x1B,
	MPL_P_MIN_MSB					= 0x1C,
	MPL_P_MIN_CSB					= 0x1D,
	MPL_P_MIN_LSB					= 0x1E,
	MPL_T_MIN_MSB					= 0x1F,
	MPL_T_MIN_LSB					= 0x20,
	MPL_P_MAX_MSB					= 0x21,
	MPL_P_MAX_CSB					= 0x22,
	MPL_P_MAX_LSB					= 0x23,
	MPL_T_MAX_MSB					= 0x24,
	MPL_T_MAX_LSB					= 0x25,
	MPL_CTRL_REG1					= 0x26,
	MPL_CTRL_REG2					= 0x27,
	MPL_CTRL_REG3					= 0x28,
	MPL_CTRL_REG4					= 0x29,
	MPL_CTRL_REG5					= 0x2A,
	MPL_OFF_P					    = 0x2B,
	MPL_OFF_T					    = 0x2C,
	MPL_OFF_H					    = 0x2D,
	MPL_SLAVE_ADDR				    = 0x60
} MPL3115A2_regaddr;

/*! \typedef Oversample ratio setting
 *
 */
typedef enum {
    MPL_OS_1                        = 0b000,//!< MPL_OS_1
    MPL_OS_2                        = 0b001,//!< MPL_OS_2
    MPL_OS_4                        = 0b010,//!< MPL_OS_4
    MPL_OS_8                        = 0b011,//!< MPL_OS_8
    MPL_OS_16                       = 0b100,//!< MPL_OS_16
    MPL_OS_32                       = 0b101,//!< MPL_OS_32
    MPL_OS_64                       = 0b110,//!< MPL_OS_64
    MPL_OS_128                      = 0b111 //!< MPL_OS_128
} MPL3115A2_os_ratio;


/*! \typedef Structure for keeping track of an MPL3115A2 transaction
 */
typedef struct mpl3115a2_driver {
    i2cflags_t           i2c_errors;
    I2CDriver*           i2c_instance;                     /*! which stm32f407 I2C instance to use (there are 3)       */
    mpl3115a2_i2c_data   txbuf[MPL3115A2_MAX_TX_BUFFER];   /*! Transmit buffer                                         */
    mpl3115a2_i2c_data   rxbuf[MPL3115A2_MAX_RX_BUFFER];   /*! Receive buffer                                          */
} MPL3115A2_Driver;

/*! \typedef  mpl3115a2 data
 *
 */
struct MPL3115A2_read_data {
	mpl3115a2_pressure_data         mpu_pressure;
	mpl3115a2_temperature_data      mpu_temperature;
} __attribute__((packed)) ;
typedef struct MPL3115A2_read_data MPL3115A2_read_data;

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
extern const mpl3115a2_connect         mpl3115a2_connections ;

extern       EventSource               mpl3115a2_data_event;
extern       EventSource               mpl3115a2_int_event;

extern       MPL3115A2_Driver          mpl3115a2_driver;
extern       MPL3115A2_read_data       mpl3115a2_current_read;

extern      const char                mplid[4];

void               mpl3115a2_start(I2CDriver* i2c) ;
void               mpl3115a2_init(I2CDriver* i2c);
msg_t              mpl3115a2_read_P_T(I2CDriver* i2c, MPL3115A2_read_data* d ) ;
mpl3115a2_i2c_data mpl3115a2_read_f_status(I2CDriver* i2c) ;
mpl3115a2_i2c_data mpl3115a2_read_ctrl_1(I2CDriver* i2c ) ;
msg_t              mpl3115a2_write_ctrl_1(I2CDriver* i2c, mpl3115a2_i2c_data rdata ) ;


#ifdef __cplusplus
}
#endif


/*!
 * @}
 */
#endif


