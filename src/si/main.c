/*! \file main.c
 *
 * Development for MPU9150 on ChibiOS
 *
 * MPU is an i2c device
 *
 * Includes ADIS SPI connections and development
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */


/*!
 * \defgroup mpu-mainapp MPU Application
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

#if !defined(DEBUG_SI) || defined(__DOXYGEN__)
#define     DEBUG_SI                   1
#endif

static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU_PSAS,
		commands
};
//
//static void log_msg(volatile char *s) {
//#if DEBUG_SI
//    static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
//    chprintf(chp, "M: %s\r\n", s);
//#else
//    (void) s;
//#endif
//}
//
//static void log_error(volatile char *s) {
//#if DEBUG_SI
//    static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
//    chprintf(chp, "E: %s\r\n", s);
//#else
//    (void) s;
//#endif
//}



static void led_init(void) {

    palClearPad(GPIOF, GPIOF_BLUE_LED);
    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_GREEN_LED);

    int i = 0;
    for(i=0; i<5; ++i) {
        palSetPad(GPIOF, GPIOF_RED_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_RED_LED);
        palSetPad(GPIOF, GPIOF_BLUE_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_BLUE_LED);
        palSetPad(GPIOF, GPIOF_GREEN_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_GREEN_LED);
    }
}

static WORKING_AREA(waThread_blinker, 64);
/*! \brief Green LED blinker thread
 */
static msg_t Thread_blinker(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");

	palClearPad(GPIOF, GPIOF_GREEN_LED);
	palClearPad(GPIOF, GPIOF_RED_LED);
	palClearPad(GPIOF, GPIOF_BLUE_LED);
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
        //palTogglePad(GPIOF, GPIOF_RED_LED);
        //palTogglePad(GPIOF, GPIOF_BLUE_LED);
        palTogglePad(GPIOF, GPIOF_GREEN_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

static WORKING_AREA(waThread_mpu9150_int, 512);
static msg_t Thread_mpu9150_int(void* arg) {
	(void) arg;
	static const evhandler_t evhndl_mpu9150[]       = {
			mpu9150_int_event_handler,
			//mpu9150_reset_req
	};
	struct EventListener     evl_mpu9150;

	chRegSetThreadName("mpu9150_int");

	mpu9150_init(mpu9150_driver.i2c_instance);

	chEvtRegister(&mpu9150_int_event,           &evl_mpu9150,         0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
	}
	return -1;
}


static WORKING_AREA(waThread_mpu9150_reset_req, 256);
static msg_t Thread_mpu9150_reset_req(void* arg) {
	(void) arg;
	static const evhandler_t evhndl_mpu9150[]       = {
			mpu9150_reset_req
	};
	struct EventListener     evl_mpu9150;

	chRegSetThreadName("mpu9150_reset_req");

	chEvtRegister(&fc_req_reset_event,          &evl_mpu9150,         0);

	while (TRUE) {
		chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
	}
	return -1;
}

static WORKING_AREA(waThread_adis_newdata, 512);
/*! \brief ADIS Newdata Thread
 */
static msg_t Thread_adis_newdata(void *arg) {
	(void)arg;
	chRegSetThreadName("adis_newdata");

	static const evhandler_t evhndl_newdata[]       = {
			adis_newdata_handler
	};
	struct EventListener     evl_spi_cb2;

	chEvtRegister(&adis_spi_cb_data_captured, &evl_spi_cb2, 0);

	while (TRUE) {
		chEvtDispatch(evhndl_newdata, chEvtWaitOneTimeout((eventmask_t)1, US2ST(50)));
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

	extdetail_init();

//	palSetPad(GPIOC, GPIOC_LED);
//	palSetPad(GPIOA, GPIOA_SPI1_SCK);
//	palSetPad(GPIOA, GPIOA_SPI1_NSS);

	/*!
	 * GPIO Pins for generating pulses at data input detect and data output send.
	 * Used for measuring latency timing of data
	 *
	 * \sa board.h
	 */
//	palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
//	palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
//	palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
//	palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );

	palClearPad(GPIOF, GPIOF_RED_LED);
	palClearPad(GPIOF, GPIOF_BLUE_LED);
	palClearPad(GPIOF, GPIOF_GREEN_LED);

	led_init();


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

	/* Administrative threads */
	chThdCreateStatic(waThread_blinker,      sizeof(waThread_blinker),      NORMALPRIO, Thread_blinker,      NULL);
	chThdCreateStatic(waThread_indwatchdog,  sizeof(waThread_indwatchdog),  NORMALPRIO, Thread_indwatchdog,  NULL);

	/* MAC */
	/*!
	 * Use a locally administered MAC address second LSbit of MSB of MAC should be 1
	 * Use unicast address LSbit of MSB of MAC should be 0
	 */
	data_udp_init();
	static       uint8_t      IMU_A_macAddress[6]         = IMU_A_MAC_ADDRESS;
	struct       ip_addr      ip, gateway, netmask;
	IMU_A_IP_ADDR(&ip);
	IMU_A_GATEWAY(&gateway);
	IMU_A_NETMASK(&netmask);

	ip_opts.macaddress = IMU_A_macAddress;
	ip_opts.address    = ip.addr;
	ip_opts.netmask    = netmask.addr;
	ip_opts.gateway    = gateway.addr;

	chThdCreateStatic(wa_lwip_thread              , sizeof(wa_lwip_thread)              , NORMALPRIO + 2, lwip_thread            , &ip_opts);
	chThdCreateStatic(wa_data_udp_send_thread     , sizeof(wa_data_udp_send_thread)     , NORMALPRIO    , data_udp_send_thread   , NULL);
	chThdCreateStatic(wa_data_udp_receive_thread  , sizeof(wa_data_udp_receive_thread)  , NORMALPRIO    , data_udp_receive_thread, NULL);

	/*! Activates the EXT driver 1. */
	extStart(&EXTD1, &extcfg);

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
