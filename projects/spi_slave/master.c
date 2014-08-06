/* Experement and demo of how to get spi slave mode to work. */
#include "ch.h"
#include "hal.h"
#include "utils_general.h"
#include "utils_led.h"

static uint16_t buf[50];

void main(void) {
	halInit();
	chSysInit();
	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST |
	                   PAL_STM32_PUDR_PULLDOWN;
	/* SPI pins setup */
	palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_OUTPUT_PUSHPULL | PINMODE);
	palSetPadMode(GPIOA, GPIOA_PIN5, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(GPIOA, GPIOA_PIN6, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(GPIOB, GPIOB_PIN5, PAL_MODE_ALTERNATE(5) | PINMODE);

	static SPIConfig spiread =
	{
		.end_cb = NULL,
		.cr1 = SPI_CR1_DFF | SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2,
		.ssport=GPIOA,
		.sspad=GPIOA_PIN4
	};
	spiStart(&SPID1, &spiread);

	for(unsigned int i = 0; i < ARRAY_SIZE(buf); ++i){
		buf[i] = i;
	}
	while(TRUE){
		spiSelect(&SPID1);
		spiSend(&SPID1, ARRAY_SIZE(buf), buf);
		spiUnselect(&SPID1);
		ledToggle(&GREEN);
		chThdSleepSeconds(1);
	}
}

