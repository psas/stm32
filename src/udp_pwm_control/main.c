/* experimental control of pwm vars over udp */

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

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"


#include "main.h"


/*shell commands*/
static const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SDU1,
		commands
};

/*blinker thread vars and functions*/
static WORKING_AREA(waThread_blinker, 64);
static msg_t Thread_blinker(void *arg) {
	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

/*watchdog timer thread vars and functions*/
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

int main(void) {
	static Thread            *shelltp       = NULL;
	/*this is needed in order for the usb over serial to work*/
	static const evhandler_t evhndl_main[]       = {
			extdetail_WKUP_button_handler
	};
	struct EventListener     el0;
	/*initialize HAL*/
	halInit();
	chSysInit();
	/*initialize serial over usb driver*/
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);
	/*activate usb driver*/
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);
	/*start shell*/
	shellInit();
	/*start watchdog*/
	iwdg_begin();
	/*start serial driver 6 and sdc driver 1 using default config*/
	sdStart(&SD6, NULL);	
	/*sleep 300 ms*/
	chThdSleepMilliseconds(300);
	/*start ext1 driver*/
	extStart(&EXTD1, &extcfg);
	/*create blinker thread*/
	chThdCreateStatic(waThread_blinker,      sizeof(waThread_blinker),      NORMALPRIO, Thread_blinker,      NULL);
	/*create watchdog thread*/
	chThdCreateStatic(waThread_indwatchdog,  sizeof(waThread_indwatchdog),  NORMALPRIO, Thread_indwatchdog,  NULL);
	/*create lwip thread*/
	chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 2, lwip_thread, &ip_opts);
	/*create udp send thread */
	chThdCreateStatic(wa_data_udp_send_thread, sizeof(wa_data_udp_send_thread), NORMALPRIO, data_udp_send_thread, NULL);
	/*create udp receive thread*/
	chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO, data_udp_receive_thread, NULL);
	while (TRUE) {
		if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		/*this is needed for the usb over serial to work */
		chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
	}

}
