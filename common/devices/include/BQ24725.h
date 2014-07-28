/*
 * BQ24725.h
 *
 * ChibiOS HAL driver for the BQ24725 battery charge controller
 */

#ifndef BQ24725_H_
#define BQ24725_H_

#include "hal.h"
#include "utils_hal.h"

typedef enum {t150ms=0, t1300ms=0x8000}  BQ24725_ACOK_deglitch_time;
#define BQ24725_ACOK_deglitch_time_MASK 0x8000

typedef enum {disabled=0, t44s=0x2000, t88s=0x4000, t175s=0x6000}
	BQ24725_WATCHDOG_timer;
#define BQ24725_WATCHDOG_timer_MASK 0x6000

typedef enum {FT59_19pct=0, FT62_65pct=0x800, FT66_55pct=0x1000,
	FT70_97pct=0x1800}	BQ24725_BAT_depletion_threshold;
#define BQ24725_BAT_depletion_threshold_MASK 0x1800

typedef enum {dec18pct=0, inc18pct=0x400}  BQ24725_EMI_sw_freq_adj;
#define BQ24725_EMI_sw_freq_adj_MASK 0x400

typedef enum {sw_freq_adj_disable=0, sw_freq_adj_enable= 0x200}
	BQ24725_EMI_sw_freq_adj_en;
#define BQ24725_EMI_sw_freq_adj_en_MASK 0x200

typedef enum {l300mV=0, l500mV=0x80, l700mV=0x100, l900mV=0x180}
	BQ24725_IFAULT_HI_threshold;
#define BQ24725_IFAULT_HI_threshold_MASK 0x180

typedef enum {LEARN_disable=0, LEARN_enable=0x40} BQ24725_LEARN_en;
#define BQ24725_LEARN_en_MASK 0x40

typedef enum {adapter_current=0, charge_current=0x20} BQ24725_IOUT;
#define BQ24725_IOUT_MASK 0x20

typedef enum {ACOC_disable=0, l1_33X=0x2, l1_66X=0x4, l2_22X=0x6}
	BQ24725_ACOC_threshold;
#define BQ24725_ACOC_threshold_MASK 0x6

typedef enum {charge_enable=0, charge_inhibit=1} BQ24725_charge_inhibit;
#define BQ24725_charge_inhibit_MASK 0x1

typedef struct BQ24725_charge_options{
	BQ24725_ACOK_deglitch_time ACOK_deglitch_time;
	BQ24725_WATCHDOG_timer WATCHDOG_timer;
	BQ24725_BAT_depletion_threshold BAT_depletion_threshold;
	BQ24725_EMI_sw_freq_adj EMI_sw_freq_adj;
	BQ24725_EMI_sw_freq_adj_en EMI_sw_freq_adj_en;
	BQ24725_IFAULT_HI_threshold IFAULT_HI_threshold;
	BQ24725_LEARN_en LEARN_en;
	BQ24725_IOUT IOUT;
	BQ24725_ACOC_threshold ACOC_threshold;
	BQ24725_charge_inhibit charge_inhibit;
} BQ24725_charge_options;

static inline uint16_t form_options_data(BQ24725_charge_options * opts){
	return opts->ACOK_deglitch_time | opts->WATCHDOG_timer |
	       opts->BAT_depletion_threshold | opts->EMI_sw_freq_adj |
	       opts->EMI_sw_freq_adj_en | opts->IFAULT_HI_threshold |
	       opts->LEARN_en | opts->IOUT | opts->ACOC_threshold |
	       opts->charge_inhibit;
}

static inline void form_options_struct(uint16_t data, BQ24725_charge_options* opt){
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

struct BQ24725Config {
	struct pin ACOK;
	extcallback_t ACOK_cb;
	I2CDriver *I2CD;
	I2CPins   *I2CP;
	ADCDriver *ADCD;
};

void BQ24725Start(struct BQ24725Config * conf);
int BQ24725_GetDeviceID(uint16_t* data);
int BQ24725_GetManufactureID(uint16_t* data);
int BQ24725_GetChargeCurrent(uint16_t* data);
int BQ24725_SetChargeCurrent(unsigned int mA);
int BQ24725_GetChargeVoltage(uint16_t* data);
int BQ24725_SetChargeVoltage(unsigned int mV);
int BQ24725_GetInputCurrent(uint16_t* data);
int BQ24725_SetInputCurrent(unsigned int mA);
int BQ24725_GetChargeOption(uint16_t* data);
int BQ24725_SetChargeOption(BQ24725_charge_options * option);
int BQ24725_ACOK(void);
int BQ24725_IMON(void);

extern const BQ24725_charge_options BQ24725_charge_options_POR_default;
#endif /* BQ24725_H_ */
