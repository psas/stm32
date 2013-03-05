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
#include "threaddetail.h"

#include "ADIS16405.h"

#include "main.h"

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
#if 1
const SPIConfig adis_spicfg = {
  adis_spi_cb,
  GPIOA,
  4,
  SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
};
#else
const SPIConfig adis_spicfg = {
  NULL,
  GPIOA,
  4,
  SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1
};

#endif
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
	iwdg_lld_set_prescale(IWDG_PS_DIV32); // This should be about 2 second at 32kHz
	iwdg_lld_reload();
	iwdg_lld_init();
}

/*
 * WKUP button handler
 *
 * Used for debugging
 */
static void WKUP_button_handler(eventid_t id) {
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;
	chprintf(chp, "\r\nWKUP btn. eventid: %d\r\n", id);
}

/*! \brief Process an adis_newdata_event
 */
static void adis_newdata_handler(eventid_t id) {
	uint8_t               i      = 0;
	static uint32_t       j      = 0;
	static uint32_t       xcount = 0;

	BaseSequentialStream    *chp = (BaseSequentialStream *)&SDU1;

	spiUnselect(adis_driver.spi_instance);                /* Slave Select de-assertion.       */

	++j;
	if(j>2000) {
		chprintf(chp, "\r\n%d rx bytes: ", xcount);
		for(i=0; i<adis_cache_data.current_rx_numbytes; ++i) {
			chprintf(chp, "0x%x ", adis_cache_data.adis_rx_cache[i]);
		}
		chprintf(chp,"\r\n");
		j=0;
		++xcount;
	}
	adis_driver.state             = ADIS_IDLE;
}

static void adis_read_id_handler(eventid_t id) {
	adis_read_id(&SPID1);
}

/*! \brief Process information from a adis_spi_cb
 *
 */
static void adis_spi_handler(eventid_t id) {
	switch(adis_driver.reg) {
		case ADIS_PRODUCT_ID:
			spiUnselect(adis_driver.spi_instance);

			/*!
			 *  This is the ADIS T_stall time.
			 *  Counting to 100 takes about 11uS.
			 *  Measured on oscilloscope.
			 *
			 *  \todo Should eventually use a timer/counter unit to generate T_stall
			 *  delays
			 */

			spiReleaseBus(adis_driver.spi_instance);
			adis_tstall_delay();
			spiAcquireBus(adis_driver.spi_instance);
			spiSelect(adis_driver.spi_instance);
			spiStartReceive(adis_driver.spi_instance, adis_driver.rx_numbytes, adis_driver.adis_rxbuf);
			adis_driver.state             = ADIS_RX_PEND;
			break;
		default:
			adis_driver.state             = ADIS_RX_PEND;
			break;
	}
}

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 64);
static msg_t Thread1(void *arg) {

	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
	    palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

static WORKING_AREA(waThread_spi_cb, 128);
static msg_t Thread_spi_cb(void *arg) {
	(void)arg;
	//BaseSequentialStream *chp = (BaseSequentialStream *)&SDU1;


	static const evhandler_t evhndl2[]       = {
			NULL,
			NULL,
			adis_newdata_handler
	};
	struct EventListener     evl_spi_cb2;

	chEvtRegister(&spi_cb_event2, &evl_spi_cb2, 2);

	while (TRUE) {
		chEvtDispatch(evhndl2, chEvtWaitOneTimeout((eventmask_t)4, US2ST(50)));
		++adis_driver.debug_cb_count;
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
static WORKING_AREA(waThread_dio1, 128);
static msg_t Thread_dio1(void *arg) {
	(void)arg;
	BaseSequentialStream *chp = (BaseSequentialStream *)&SDU1;

	static const evhandler_t evhndl_spi1[]       = {
			adis_read_id_handler,
			adis_spi_handler,
			adis_release_bus
	};
	struct EventListener     evl_dio;
	struct EventListener     evl_spi_ev;
	struct EventListener     evl_spi_release;

	chRegSetThreadName("adis_dio");

	chEvtRegister(&dio1_event,       &evl_dio, 0);
	chEvtRegister(&spi_cb_event,     &evl_spi_ev, 1);
	chEvtRegister(&spi_cb_event2,    &evl_spi_release, 2);

	while (TRUE) {
		chEvtDispatch(evhndl_spi1, chEvtWaitOneTimeout((EVENT_MASK(2)|EVENT_MASK(1)|EVENT_MASK(0)), US2ST(50)));
		//chprintf(chp, "%d %d\r\n", adis_driver.debug_cb_count, adis_driver.debug_spi_count);
	}
	return -1;
}

/*
 * Watchdog thread
 */
static WORKING_AREA(waThread_indwatchdog, 64);
static msg_t Thread_indwatchdog(void *arg) {
	(void)arg;

	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(250);
	}
	return -1;
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
	chEvtInit(&dio1_event);
	chEvtInit(&spi_cb_event);
	chEvtInit(&spi_newdata_event);
	chEvtInit(&spi_cb_event2);

	palSetPad(GPIOC, GPIOC_LED);
	palSetPad(GPIOA, GPIOA_SPI1_SCK);
	palSetPad(GPIOA, GPIOA_SPI1_NSS);

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
	palSetPad(GPIOA, GPIOA_SPI1_NSS);

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

	spiStart(&SPID1, &adis_spicfg);       /* Setup transfer parameters.       */

	chThdSleepSeconds(1);

	adis_init();
	adis_reset();
	/*!
	 * Activates the EXT driver 1.
	 * This is for the external interrupt
	 */
	extStart(&EXTD1, &extcfg);

	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	chThdCreateStatic(waThread_dio1, sizeof(waThread_dio1), NORMALPRIO, Thread_dio1, NULL);
	chThdCreateStatic(waThread_spi_cb, sizeof(waThread_spi_cb), NORMALPRIO, Thread_spi_cb, NULL);
	chThdCreateStatic(waThread_indwatchdog, sizeof(waThread_indwatchdog), NORMALPRIO, Thread_indwatchdog, NULL);

	chEvtRegister(&wkup_event, &el0, 0);
	while (TRUE) {
		if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chEvtDispatch(evhndl, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}
