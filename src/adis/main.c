/*! \file main.c
 *
 * Development for ADIS IMU on ChibiOS
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "usb_cdc.h"
#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"

#include "ADIS16405.h"

#include "main.h"


EventSource   wkup_event;
EventSource   spi1_event;


static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU1,
		commands
};

/*! \brief ADIS SPI configuration
 *
 * 656250Hz, CPHA=1, CPOL=1, MSb first.
 *
 * For burst mode ADIS SPI is limited to 1Mhz.
 */
//const SPIConfig adis_spicfg = {
//  adis_spi_cb,
//  GPIOA,
//  4,
//  SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
//};
const SPIConfig adis_spicfg = {
  NULL,
  GPIOA,
  4,
  SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
};

/*
 * Maximum speed SPI configuration (21MHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig hs_spicfg = {
  NULL,
  GPIOA,
  4,
  0
};

/*! ADIS SPI Pin connections
 *
 */
const adis_connect adis_connections = {
	GPIOD,      // reset_port
	8,          // reset_pad;
	GPIOD,      // dio1_port;
	9,          // dio1_pad;
	GPIOD,      // dio2_port;
	10,         // dio2_pad;
	GPIOD,      // dio3_port;
	11,         // dio3_pad;
	GPIOD,      // dio4_port;
    12          // dio4_pad
};

/*
 * WKUP button handler
 *
 */
static void WKUP_button_handler(eventid_t id) {
	uint8_t i = 0;
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;
	chprintf(chp, "\r\nWKUP btn. eventid: %d\r\n", id);
	chprintf(chp, "spi1->cr1: 0x%x\r\n", SPI1->CR1);
	chprintf(chp, "spi1->cr2: 0x%x\r\n", SPI1->CR2);
	chprintf(chp, "spi1->sr: 0x%x\r\n", SPI1->SR);
	chprintf(chp, "spi1->dr: 0x%x\r\n", SPI1->DR);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream0);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream1);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream2);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream3);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream4);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream5);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream6);
	chprintf(chp, "dmastream0: 0x%x\r\n", DMA2_Stream7);
    for(i=0; i< 10; ++i) {
    	chprintf(chp, "0x%x ", adis_driver.adis_rxbuf[i]);
    }
    chprintf(chp, "\r\n");
}

/*!
 * spi1_event handler
 *
 * Start an asynchronous spi transaction
 *
 * spi1_event is an DIO1 data ready interrupt from ADIS
 *
 */
static void SPI1_handler(eventid_t id) {
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;
	chprintf(chp, "SPI1. eventid: %d\r\n", id);

	chThdSleepMilliseconds(300);
	//adis_read_id(&SPID1);
	spi_test(&SPID1);

}

static void adis_handler(eventid_t id) {
	uint8_t                 i = 0;
	BaseSequentialStream *chp = (BaseSequentialStream *)&SDU1;
	chprintf(chp, "adis_newdata. eventid: %d\r\n", id);

	for(i=0; i<adis_cache_data.current_rx_numbytes; ++i) {
		chprintf(chp, "%d", adis_cache_data.adis_rx_cache[i]);
	}
	chprintf(chp,"\r\n");
}

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

/*
 * SPI1 thread
 *
 * For burst mode transactions t_readrate is 1uS
 * Waking every 5 us to look for interrupt seems ok for a start.
 *
 */
static WORKING_AREA(waThread2, 128);
static msg_t Thread2(void *arg) {
	(void)arg;
	static const evhandler_t evhndl_spi1[]       = {
			SPI1_handler,
			adis_handler
	};
	struct EventListener     evl_spi0;
	struct EventListener     evl_spi1;

	chRegSetThreadName("spi1_adis");

	chEvtRegister(&spi1_event,         &evl_spi0, 0);
	chEvtRegister(&adis_newdata_event, &evl_spi1, 1);

	while (TRUE) {
		chThdSleepMicroseconds(5);
		chEvtDispatch(evhndl_spi1, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(20)));
	}
	return -1;
}

/*
 * Watchdog thread
 */
static WORKING_AREA(waThread3, 64);
static msg_t Thread3(void *arg) {
	(void)arg;

	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(250);
	}
	return -1;
}

/*! \brief check reset status and then start iwatchdog
 *
 * Check the CSR register for reset source then start
 * the independent watchdog counter.
 *
 */
static void begin_iwdg(void) {
	// was this a reset caused by the iwdg?
	if( (RCC->CSR & RCC_CSR_WDGRSTF) != 0) {
		// \todo Log WDG reset event somewhere.
		RCC->CSR = RCC->CSR | RCC_CSR_RMVF;  // clear the IWDGRSTF
	}
	iwdg_lld_set_prescale(IWDG_PS_DIV16); // This should be about 2 second at 32kHz
	iwdg_lld_reload();
	iwdg_lld_init();
}

/*
 * Application entry point.
 */
int main(void) {
	static Thread            *shelltp       = NULL;
	static const evhandler_t evhndl[]       = {
			WKUP_button_handler
	};
	struct EventListener     el0;

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	chEvtInit(&wkup_event);
	chEvtInit(&spi1_event);
	chEvtInit(&adis_newdata_event);


	palSetPad(GPIOA, GPIOA_SPI1_SCK);
	chThdSleepMilliseconds(50);
	palClearPad(GPIOA, GPIOA_SPI1_SCK);
	chThdSleepMilliseconds(50);
	palSetPad(GPIOA, GPIOA_SPI1_SCK);
	chThdSleepMilliseconds(50);
	palClearPad(GPIOA, GPIOA_SPI1_SCK);
	chThdSleepMilliseconds(50);


	/*
	 * SPI1 I/O pins setup.
	 */
	palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5) |
			PAL_STM32_OSPEED_HIGHEST);       /* New SCK.     */
	palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5) |
			PAL_STM32_OSPEED_HIGHEST| PAL_STM32_PUDR_FLOATING);       /* New MISO.    */
	palSetPadMode(GPIOB, 5, PAL_MODE_ALTERNATE(5) |
			PAL_STM32_OSPEED_HIGHEST );       /* New MOSI.    */
	palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL |
			PAL_STM32_OSPEED_HIGHEST);       /* New CS.      */
	palSetPad(GPIOA, 4);



	/*!
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

	/*!
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	shellInit();

	begin_iwdg();

	/*!
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	adis_init();
	adis_reset();

	/*!
	 * Activates the EXT driver 1.
	 * This is for the external interrupt
	 */
	extStart(&EXTD1, &extcfg);

	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
	chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);

	chEvtRegister(&wkup_event, &el0, 0);
	while (TRUE) {
		if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
	}
}
