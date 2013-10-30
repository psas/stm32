/*
 * BQ24725.c
 * ChibiOS HAL driver for using the BQ24725 Battery Charge Controller from the RocketNet Hub
 */

#include <stdint.h>
#include <stdlib.h> //for NULL
#include <stdbool.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "BQ24725.h"
#include "stm32f4xx.h"

static I2CDriver *I2CD = &I2CD1;
static bool initialized = false;

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

inline uint16_t form_options_data(BQ24725_charge_options * opts){
    return opts->ACOK_deglitch_time | opts->WATCHDOG_timer |
            opts->BAT_depletion_threshold | opts->EMI_sw_freq_adj |
            opts->EMI_sw_freq_adj_en | opts->IFAULT_HI_threshold |
            opts->LEARN_en | opts->IOUT | opts->ACOC_threshold |
            opts->charge_inhibit;
}
inline void form_options_struct(uint16_t data, BQ24725_charge_options* opt){
    opt->ACOK_deglitch_time = data & BQ24725_ACOK_deglitch_time_MASK;
    opt->WATCHDOG_timer = data & BQ24725_WATCHDOG_timer_MASK;
    opt->BAT_depletion_threshold = data & BQ24725_BAT_depletion_threshold_MASK;
    opt->EMI_sw_freq_adj = data & BQ24725_EMI_sw_freq_adj_MASK;
    opt->EMI_sw_freq_adj_en = data & BQ24725_EMI_sw_freq_adj_en_MASK;
    opt->IFAULT_HI_threshold = data & BQ24725_IFAULT_HI_threshold_MASK;
    opt->LEARN_en = data & BQ24725_LEARN_en_MASK;
    opt->IOUT = data & BQ24725_IOUT_MASK;
    opt->ACOC_threshold = data & BQ24725_ACOC_threshold_MASK;
    opt->charge_inhibit = data & BQ24725_charge_inhibit_MASK;
}

static const I2CConfig i2cfg = {
	OPMODE_SMBUS_HOST,
    100000,
    STD_DUTY_CYCLE,
};

static void BQ24725_init(void){
    if(initialized == false)
        i2cStart(I2CD, &i2cfg);
        initialized = true;
	//todo: gpio for acok?
	//todo: adc for IMON?
}

//todo: form options data/struct
static int BQ24725_Get(uint8_t register_id, uint16_t* data){
    if(initialized == false)
        BQ24725_init();

    uint8_t tx[1] = {register_id};
    uint8_t rx[2];
    i2cflags_t errors;
	i2cAcquireBus(I2CD);
	msg_t status = i2cMasterTransmitTimeout(I2CD, BQ24725_ADDR, tx, sizeof(tx), rx, sizeof(rx), 10);
	switch(status){
	case RDY_OK:
		i2cReleaseBus(I2CD);
		break;
	case RDY_RESET:
		errors = i2cGetErrors(I2CD);
		//todo: will error tell if i2c not init?
		i2cReleaseBus(I2CD);
		return errors;
	case RDY_TIMEOUT:
		i2cReleaseBus(I2CD);
		return -2;
	default:
		i2cReleaseBus(I2CD);
	}

	*data = DATA_FROM_BYTES(rx[0], rx[1]);
	return 0;
}

static int BQ24725_Set(uint8_t register_id, uint16_t data){
    if(initialized == false)
    	BQ24725_init();

    uint8_t tx[3] = {register_id, LOWDATA_BYTE(data), HIGHDATA_BYTE(data)};

	i2cAcquireBus(I2CD);
	msg_t status = i2cMasterTransmitTimeout(I2CD, BQ24725_ADDR, tx, sizeof(tx), NULL, 0, 10);
	if (status != RDY_OK){
		i2cflags_t errors = i2cGetErrors(I2CD);
		i2cReleaseBus(I2CD);
		return errors;
	}
	i2cReleaseBus(I2CD);

	return 0;
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
	return palReadPad(GPIOD, GPIO_D0_BQ24_ACOK);
}

int BQ24725_IMON(void){
	return 0;
}

//void BQ24725_UpdateChargeOption(uint16_t option){
//
//}

