#include <stddef.h>
#include "ch.h"
#include "hal.h"
#include "mii.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_hal.h"
#include "utils_led.h"

uint8_t macaddr[6] = {0xE6, 0x10, 0x20, 0x30, 0x40, 0x11};
const MACConfig macconf = {macaddr};

void led2num(int i){
	(i & 1? ledOn:ledOff)(&LED2);
	(i & 2? ledOn:ledOff)(&LED4);
	(i & 4? ledOn:ledOff)(&LED5);
}

void main(void) {

	halInit();
	chSysInit();

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

	while(TRUE){
		for(int i = 0; i < 10; ++i)
			palSetPadMode(test[i].port, test[i].pad, PAL_MODE_OUTPUT_PUSHPULL);
		palSetPadMode(test[10].port, test[10].pad, PAL_MODE_INPUT);

		for(int i = 0; i < 10; ++i)
			palSetPad(test[i].port, test[i].pad);

		chThdSleepMilliseconds(100);
		unsigned int val = palReadPad(test[10].port, test[10].pad);

		for(int i = 0; i < 10; ++i){
			led2num(i);
			ledOn(&BLUE);
			chThdSleepMilliseconds(500);
			ledOff(&BLUE);
			palClearPad(test[i].port, test[i].pad);
			chThdSleepMilliseconds(100);
			if(val != palReadPad(test[10].port, test[10].pad)){
				ledOn(&GREEN);
			} else {
				ledOn(&RED);
			}

			val = palReadPad(test[10].port, test[10].pad);
			chThdSleepMilliseconds(500);
			ledOff(&GREEN);
			ledOff(&RED);
		}
		chThdSleepMilliseconds(1000);
	}
}

