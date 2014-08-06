/* Experement and demo of how to get spi slave mode to work. */
#include "ch.h"
#include "hal.h"
#include "utils_general.h"
#include "utils_led.h"

static uint16_t buf[50];

static EVENTSOURCE_DECL(doneread);

static void spiread(void){
	spiStartReceive(&SPID1, ARRAY_SIZE(buf), buf);
}
static void spireadcb(SPIDriver *spip UNUSED){
	chSysLockFromIsr();
	spiStartReceiveI(&SPID1, ARRAY_SIZE(buf), buf);
	chEvtBroadcastI(&doneread);
	chSysUnlockFromIsr();
}

static void readsuccessful(eventid_t e UNUSED){
	ledToggle(&GREEN);
}

void main(void) {
	halInit();
	chSysInit();
	uint32_t PINMODE = PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST |
	                   PAL_STM32_PUDR_FLOATING;
	/* SPI pins setup */
	palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_INPUT | PINMODE);
	palSetPadMode(GPIOA, GPIOA_PIN5, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(GPIOA, GPIOA_PIN6, PAL_MODE_ALTERNATE(5) | PINMODE);
	palSetPadMode(GPIOB, GPIOB_PIN5, PAL_MODE_ALTERNATE(5) | PINMODE);

	static SPIConfig spicfg =
	{
		.end_cb = spireadcb,
		.cr1 = SPI_CR1_DFF
	};
	spiStart(&SPID1, &spicfg);
	SPID1.spi->CR1 &= ~SPI_CR1_SPE;
	SPID1.spi->CR1 &= ~(SPI_CR1_MSTR | SPI_CR1_SSI);
	SPID1.spi->CR1 |= SPI_CR1_SSM | SPI_CR1_RXONLY;
	SPID1.spi->CR1 |= SPI_CR1_SPE;

	struct EventListener el;
	chEvtRegister(&doneread, &el, 0);

	const evhandler_t evhndl[] = {
		readsuccessful
	};

	spiread();
	while(TRUE){
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}

