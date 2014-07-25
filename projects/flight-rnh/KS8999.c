/*
 * KS8999.c
 */

#include "hal.h"
#include "KS8999.h"

void KS8999Start(void) {
	palSetPad(GPIOD, GPIO_D14_KSZ_EN);   // enable power
	KS8999_clock(TRUE);                  // enable clock
	palSetPad(GPIOD, GPIO_D4_ETH_N_RST); // disable reset
}

void KS8999_enable(int enable){
	if(enable){
		KS8999_power(TRUE);
		palSetPad(GPIOD, GPIO_D4_ETH_N_RST);
	}else{
		palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
		KS8999_power(FALSE);
	}
}

void KS8999_power(int enable){
	if(enable){
		palSetPad(GPIOD, GPIO_D14_KSZ_EN);
		KS8999_clock(TRUE);
	}else{
		KS8999_clock(FALSE);
		palClearPad(GPIOD, GPIO_D14_KSZ_EN);
	}
}

void KS8999_clock(int enable){
	if(enable){
		palSetPadMode(GPIOC, GPIO_C9_KSZ_25MHZ, PAL_MODE_ALTERNATE(0));
	}else{
		palSetPadMode(GPIOC, GPIO_C9_KSZ_25MHZ, PAL_MODE_RESET);
	}
}
