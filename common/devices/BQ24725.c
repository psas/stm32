/*
 * BQ24725.c
 * ChibiOS HAL driver for using the BQ24725 Battery Charge Controller
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_hal.h"
#include "BQ24725.h"

#define BQ24725_ADDR    0b0001001

typedef enum {
	DEVICE_ID       = 0xFF,
	MANUFACTURE_ID  = 0xFE,
	CHARGE_CURRENT  = 0x14,
	CHARGE_VOLTAGE  = 0x15,
	INPUT_CURRENT   = 0x3F,
	CHARGE_OPTION   = 0x12
} BQ24725_register;

#define CHARGE_CURRENT_MASK 0x1FC0
#define CHARGE_VOLTAGE_MASK 0x7FF0
#define INPUT_CURRENT_MASK  0x1F80

static struct BQ24725Config * CONF;
static bool initialized = false;
static uint16_t imonbuf = 0;

const BQ24725_charge_options BQ24725_charge_options_POR_default = {
	.ACOK_deglitch_time = t150ms,
	.WATCHDOG_timer = t175s,
	.BAT_depletion_threshold = FT70_97pct,
	.EMI_sw_freq_adj = dec18pct,
	.EMI_sw_freq_adj_en = sw_freq_adj_disable,
	.IFAULT_HI_threshold = l700mV,
	.LEARN_en = LEARN_disable,
	.IOUT = adapter_current,
	.ACOC_threshold = l1_66X,
	.charge_inhibit = charge_enable
};

void BQ24725Start(struct BQ24725Config * conf){
	chDbgAssert(conf, DBG_PREFIX"No configuration given to BQ24725 init", NULL);
	// todo: What should re-initialization look like?
	chDbgAssert(!initialized, DBG_PREFIX"BQ24725 already initialized", NULL);

	CONF = conf;

	static const I2CConfig i2cfg = {
		OPMODE_SMBUS_HOST,
		100000,
		STD_DUTY_CYCLE,
	};
	i2cUtilsStart(CONF->I2CD, &i2cfg, conf->I2CP);

	palSetPadMode(CONF->ACOK.port, CONF->ACOK.pad, PAL_MODE_INPUT_PULLDOWN | PAL_STM32_OSPEED_HIGHEST);
	extAddCallback(&(CONF->ACOK), EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART, CONF->ACOK_cb);
	extUtilsStart();

	adcStart(CONF->ADCD, &(ADCConfig){0});

	static ADCConversionGroup group= {
		.circular = TRUE,
		.num_channels = 1,
		.end_cb = NULL,
		.error_cb = NULL,
		.cr1 = 0,
		.cr2 = ADC_CR2_SWSTART,
		.smpr1 = 0,
		.smpr2 = ADC_SMPR2_SMP_AN2 (ADC_SAMPLE_480),
		.sqr1 = ADC_SQR1_NUM_CH(1),
		.sqr2 = 0,
		.sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN2)
	};
	adcStartConversion(CONF->ADCD, &group, &imonbuf, 1);
	initialized = true;
}

static int BQ24725_Get(uint8_t register_id, uint16_t* data){
	chDbgAssert(initialized, DBG_PREFIX"BQ24725 driver not initialized", NULL);
	switch(register_id){
	case DEVICE_ID:
	case MANUFACTURE_ID:
	case CHARGE_CURRENT:
	case CHARGE_VOLTAGE:
	case INPUT_CURRENT:
	case CHARGE_OPTION:
		return SMBusGet(CONF->I2CD, BQ24725_ADDR, register_id, data);
	default:
		chDbgPanic(DBG_PREFIX"Unrecognized BQ24725 read register ID");
	}
	return RDY_RESET;
}

static int BQ24725_Set(uint8_t register_id, uint16_t data){
	chDbgAssert(initialized, DBG_PREFIX"BQ24725 driver not initialized", NULL);
	switch(register_id){
	case CHARGE_CURRENT:
	case CHARGE_VOLTAGE:
	case INPUT_CURRENT:
	case CHARGE_OPTION:
		return SMBusSet(CONF->I2CD, BQ24725_ADDR, register_id, data);
	default:
		chDbgPanic(DBG_PREFIX"Unrecognized BQ24725 write register ID");
	}
	return RDY_RESET;
}

int BQ24725_GetDeviceID(uint16_t* data){
	return BQ24725_Get(DEVICE_ID, data);
}

int BQ24725_GetManufactureID(uint16_t* data){
	return BQ24725_Get(MANUFACTURE_ID, data);
}

int BQ24725_GetChargeCurrent(uint16_t* data){
	return BQ24725_Get(CHARGE_CURRENT, data);
}

int BQ24725_SetChargeCurrent(unsigned int mA){
	uint16_t data = mA & CHARGE_CURRENT_MASK;
	return BQ24725_Set(CHARGE_CURRENT, data);
}

int BQ24725_GetChargeVoltage(uint16_t* data){
	return BQ24725_Get(CHARGE_VOLTAGE, data);
}

int BQ24725_SetChargeVoltage(unsigned int mV){
	uint16_t data = mV & CHARGE_VOLTAGE_MASK;
	return BQ24725_Set(CHARGE_VOLTAGE, data);
}

int BQ24725_GetInputCurrent(uint16_t* data){
	return BQ24725_Get(INPUT_CURRENT, data);
}

int BQ24725_SetInputCurrent(unsigned int mA){
	uint16_t data = mA & INPUT_CURRENT_MASK;
	return BQ24725_Set(INPUT_CURRENT, data);
}

int BQ24725_GetChargeOption(uint16_t* data){
	return BQ24725_Get(CHARGE_OPTION, data);
}

int BQ24725_SetChargeOption(BQ24725_charge_options * option){
	uint16_t data = form_options_data(option);
	return BQ24725_Set(CHARGE_OPTION, data);
}

int BQ24725_ACOK(void){
	return palReadPad(CONF->ACOK.port, CONF->ACOK.pad);
}

int BQ24725_IMON(void){
	return imonbuf;
}

