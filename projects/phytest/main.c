#include <stddef.h>
#include "ch.h"
#include "hal.h"
#include "mii.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_hal.h"
#include "utils_led.h"

/* MII divider optimal value.*/
#if (STM32_HCLK >= 150000000)
#define MACMIIDR_CR ETH_MACMIIAR_CR_Div102
#elif (STM32_HCLK >= 100000000)
#define MACMIIDR_CR ETH_MACMIIAR_CR_Div62
#elif (STM32_HCLK >= 60000000)
#define MACMIIDR_CR		ETH_MACMIIAR_CR_Div42
#elif (STM32_HCLK >= 35000000)
#define MACMIIDR_CR		ETH_MACMIIAR_CR_Div26
#elif (STM32_HCLK >= 20000000)
#define MACMIIDR_CR		ETH_MACMIIAR_CR_Div16
#else
#error "STM32_HCLK below minimum frequency for ETH operations (20MHz)"
#endif

/**
 * @brief	Writes a PHY register.
 *
 * @param[in] macp		pointer to the @p MACDriver object
 * @param[in] reg		register number
 * @param[in] value		new register value
 */
static void mii_write(MACDriver *macp, uint32_t reg, uint32_t value) {

	ETH->MACMIIDR = value;
	ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR |
					ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;
	while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
		;
}
/**
 * @brief   Reads a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 *
 * @return              The PHY register content.
 */
static uint32_t mii_read(MACDriver *macp, uint32_t reg) {
  ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR | ETH_MACMIIAR_MB;
  while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
    ;
  return ETH->MACMIIDR;
}

static struct led_config led_cfg = {
	.cycle_ms = 250,
	.start_ms = 4000,
	.led = (const struct led*[]){
		&LED2,
		&LED4,
		&LED5,
		NULL
	}
};

uint8_t macaddr[6] = {0xE6, 0x10, 0x20, 0x30, 0x40, 0x11};
const MACConfig macconf = {macaddr};

void led2num(int i){
	ledOff(&LED2);
	ledOff(&LED4);
	ledOff(&LED5);
	if(i & 1)
		ledOn(&LED2);
	if(i & 2)
		ledOn(&LED4);
	if(i & 4)
		ledOn(&LED5);
}

void main(void) {

	halInit();
	chSysInit();

//	ledStart(&led_cfg);
	macStart(&ETHD1, &macconf);

	mii_write(&ETHD1, 0x16, mii_read(&ETHD1, 0x16) | (1<<5));

	struct pin test[11] = {
		{GPIOA, GPIOA_ETH_RMII_MDIO},
		{GPIOC, GPIOC_ETH_RMII_MDC},
		{GPIOC, GPIOC_ETH_RMII_RXD1},
		{GPIOC, GPIOC_ETH_RMII_RXD0},
		{GPIOA, GPIOA_ETH_RMII_CRS_DV},
		{GPIOA, GPIOA_ETH_RMII_REF_CLK},
		{GPIOA, GPIOA_ETH_RMII_MDINT},
		{GPIOB, GPIOB_ETH_RMII_TX_EN},
		{GPIOE, GPIOE_PIN2},
		{GPIOB, GPIOB_ETH_RMII_TXD0},
		{GPIOB, GPIOB_ETH_RMII_TXD1}
	};

	for(int i = 0; i < 10; ++i)
		palSetPadMode(test[i].port, test[i].pad, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(test[10].port, test[10].pad, PAL_MODE_INPUT);

	for(int i = 0; i < 10; ++i)
		palSetPad(test[i].port, test[i].pad);

	chThdSleepMilliseconds(100);
	unsigned int val = palReadPad(test[10].port, test[10].pad);

	for(int i = 0; i < 10; ++i){
		led2num(i);
		ledOn(&GREEN);
		chThdSleepMilliseconds(500);
		ledOff(&GREEN);
		palClearPad(test[i].port, test[i].pad);
		chThdSleepMilliseconds(100);
		if(val != palReadPad(test[10].port, test[10].pad)){
			ledOn(&BLUE);
		}

		val = palReadPad(test[10].port, test[10].pad);
		chThdSleepMilliseconds(500);
		ledOff(&BLUE);
	}

	while(TRUE){
		chThdSleep(TIME_INFINITE);
	}

}

