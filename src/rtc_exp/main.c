/*! \file main.c
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp RTC experiments
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "board.h"

#include "device_net.h"
#include "fc_net.h"

#include "main.h"


static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{"date",  cmd_date},
		{"phy", cmd_phy},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU_PSAS,
		commands
};

static WORKING_AREA(waThread_blinker, 64);
/*! \brief Green LED blinker thread
 */
static msg_t Thread_blinker(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

static WORKING_AREA(waThread_indwatchdog, 64);
/*! \brief  Watchdog thread
 */
static msg_t Thread_indwatchdog(void *arg) {
	(void)arg;

	chRegSetThreadName("iwatchdog");
	while (TRUE) {
		iwdg_lld_reload();
		chThdSleepMilliseconds(250);
	}
	return -1;
}

/**
 * @brief   Beginning of configuration procedure.
 *
 * @notapi
 */
#define rtc_lld_enter_init() {                                                \
  RTCD1.id_rtc->ISR |= RTC_ISR_INIT;                                          \
  while ((RTCD1.id_rtc->ISR & RTC_ISR_INITF) == 0)                            \
    ;                                                                         \
}
/**
 * @brief   Finalizing of configuration procedure.
 *
 * @notapi
 */
#define rtc_lld_exit_init() {RTCD1.id_rtc->ISR &= ~RTC_ISR_INIT;}

/*!
 * \brief   Enable access to registers.
 *
 * This application will use the subsecond register
 * at full speed for sub second timestamping.
 *
 * This requires the prescaler registers to be set
 * differently from the ChibiOS default values.
 */
void psas_rtc_lld_init(void){
  RTCD1.id_rtc = RTC;

  /*
   * This prescaler is run from the LSIRC at 32khz
   */
  uint32_t prediv_a = 0x1;  // this is the asynch 7 bit register

  /* Disable write protection. */
  RTCD1.id_rtc->WPR = 0xCA;
  RTCD1.id_rtc->WPR = 0x53;
  RTCD1.id_rtc->CR   |= RTC_CR_COE;
     RTCD1.id_rtc->CR   |= RTC_CR_COSEL;

  /* If calendar not init yet. */
  if (!(RTC->ISR & RTC_ISR_INITS)){
    rtc_lld_enter_init();

    /*
     * Prescaler register must be written in two SEPARATE writes.
     * See page 630 of reference manual
     *
     * To have 1Hz output for ck_spre set
     * preset_s = (STM32_RTCCLK / (prediv_a + 1)) - 1)
     */
    prediv_a = (prediv_a << 16) |
                (((STM32_RTCCLK / (prediv_a + 1)) - 1) & 0x7FFF);
    RTCD1.id_rtc->PRER = prediv_a;
    RTCD1.id_rtc->PRER = prediv_a;


    rtc_lld_exit_init();
  }
}

int main(void) {
	static Thread            *shelltp       = NULL;
	static const evhandler_t evhndl_main[]       = {
			extdetail_WKUP_button_handler
	};
	struct EventListener     el0;

	struct lwipthread_opts   ip_opts;

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	psas_rtc_lld_init();

	extdetail_init();

	palSetPad(GPIOC, GPIOC_LED);

	/*!
	 * GPIO Pins for generating pulses at data input detect and data output send.
	 * Used for measuring latency timing of data
	 *
	 * \sa board.h
	 */
	palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
	palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
	palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );

	/*!
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU_PSAS);
	sduStart(&SDU_PSAS, &serusbcfg);

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

	iwdg_begin();

	/*!
	 * Activates the serial driver 6 and SDC driver 1 using default
	 * configuration.
	 */
	sdStart(&SD6, NULL);

	chThdSleepMilliseconds(300);

	/*! Activates the EXT driver 1. */
	extStart(&EXTD1, &extcfg);

	//chThdCreateStatic(waThread_blinker          , sizeof(waThread_blinker)          , NORMALPRIO    , Thread_blinker         , NULL);
	chThdCreateStatic(waThread_indwatchdog      , sizeof(waThread_indwatchdog)      , NORMALPRIO    , Thread_indwatchdog     , NULL);

	static       uint8_t      IMU_macAddress[6]           = IMU_A_MAC_ADDRESS;
	struct       ip_addr      ip, gateway, netmask;
	IMU_A_IP_ADDR(&ip);
	IMU_A_GATEWAY(&gateway);
	IMU_A_NETMASK(&netmask);

	ip_opts.macaddress = IMU_macAddress;
	ip_opts.address    = ip.addr;
	ip_opts.netmask    = netmask.addr;
	ip_opts.gateway    = gateway.addr;

	chThdCreateStatic(wa_lwip_thread            , sizeof(wa_lwip_thread)            , NORMALPRIO + 2, lwip_thread            , &ip_opts);
    chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);

	chEvtRegister(&extdetail_wkup_event, &el0, 0);

	while (TRUE) {
		if (!shelltp && (SDU_PSAS.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}
}


//! @}
