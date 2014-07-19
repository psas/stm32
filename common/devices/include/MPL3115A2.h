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

#include "utils_hal.h"

#define MPL3115A2_CTL1_ALT_BIT 7
#define MPL3115A2_CTL1_SBYB_BIT 0
#define MPL3115A2_CTL1_OST_BIT 1
#define MPL3115A2_CTL1_RST_BIT 2
#define MPL3115A2_CTL1_OS_BITS 3
#define MPL3115A2_CTRL4_DRDY_INT_BIT 7
#define MPL3115A2_CTRL5_INT_CFG_DRDY 7

typedef enum  {
	MPL_STATUS          = 0x00,
	MPL_OUT_P_MSB       = 0x01,
	MPL_OUT_P_CSB       = 0x02,
	MPL_OUT_P_LSB       = 0x03,
	MPL_OUT_T_MSB       = 0x04,
	MPL_OUT_T_LSB       = 0x05,
	MPL_DR_STATUS       = 0x06,
	MPL_OUT_P_DELTA_MSB = 0x07,
	MPL_OUT_P_DELTA_CSB = 0x08,
	MPL_OUT_P_DELTA_LSB = 0x09,
	MPL_OUT_T_DELTA_MSB = 0x0A,
	MPL_OUT_T_DELTA_LSB = 0x0B,
	MPL_WHO_AM_I        = 0x0C,
	MPL_F_STATUS        = 0x0D,
	MPL_F_DATA          = 0x0E,
	MPL_F_SETUP         = 0x0F,
	MPL_TIME_DLY        = 0x10,
	MPL_SYSMOD          = 0x11,
	MPL_INT_SOURCE      = 0x12,
	MPL_PT_DATA_CFG     = 0x13,
	MPL_BAR_IN_MSB      = 0x14,
	MPL_BAR_IN_LSB      = 0x15,
	MPL_P_TGT_MSB = 0x16,
	MPL_P_TGT_LSB = 0x17,
	MPL_T_TGT     = 0x18,
	MPL_P_WND_MSB = 0x19,
	MPL_P_WND_LSB = 0x1A,
	MPL_T_WND     = 0x1B,
	MPL_P_MIN_MSB = 0x1C,
	MPL_P_MIN_CSB = 0x1D,
	MPL_P_MIN_LSB = 0x1E,
	MPL_T_MIN_MSB = 0x1F,
	MPL_T_MIN_LSB = 0x20,
	MPL_P_MAX_MSB = 0x21,
	MPL_P_MAX_CSB = 0x22,
	MPL_P_MAX_LSB = 0x23,
	MPL_T_MAX_MSB = 0x24,
	MPL_T_MAX_LSB = 0x25,
	MPL_CTRL_REG1 = 0x26,
	MPL_CTRL_REG2 = 0x27,
	MPL_CTRL_REG3 = 0x28,
	MPL_CTRL_REG4 = 0x29,
	MPL_CTRL_REG5 = 0x2A,
	MPL_OFF_P     = 0x2B,
	MPL_OFF_T     = 0x2C,
	MPL_OFF_H     = 0x2D,
	MPL_SLAVE_ADD = 0x60
} MPL3115A2_regaddr;

/*! \typedef Oversample ratio setting
 *
 */
typedef enum {
	MPL_OS_1   = 0b000,
	MPL_OS_2   = 0b001,
	MPL_OS_4   = 0b010,
	MPL_OS_8   = 0b011,
	MPL_OS_16  = 0b100,
	MPL_OS_32  = 0b101,
	MPL_OS_64  = 0b110,
	MPL_OS_128 = 0b111
} MPL3115A2_os_ratio;


/*! \typedef  mpl3115a2 data
 *
 */
struct MPL3115A2Data {
	uint8_t status;
	uint32_t pressure;
	int16_t temperature;
};

/*! \typedef mpl3115a2_config
 *
 * Configuration for the MPU IMU connections
 */
struct MPL3115A2Config {
	I2CDriver * i2cd;
	I2CPins pins;
	struct pin interrupt;
};

extern EventSource MPL3115A2DataEvt;

void MPL3115A2Start(struct MPL3115A2Config * conf);
void MPL3115A2GetData(struct MPL3115A2Data * data);
#endif


