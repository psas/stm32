/* experiments with UDP
 */

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#include "lwipthread.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_shell.h"
#include "utils_led.h"

#include "usbdetail.h"
#include "enet_api.h"
#include "net_addrs.h"

ManualData ManualStatus;
Neutral NeutralStatus;

static EVENTSOURCE_DECL(ReadyNeutral);
static EVENTSOURCE_DECL(ReadyManual);
static EVENTSOURCE_DECL(ReadySLA);

static int count = 0;

/*
 * GMD control thread, times are in microseconds.
 */
static void ethernet_loop(GPTDriver *gptp UNUSED) {
	chSysLockFromIsr();

	chEvtBroadcastI(&ReadyNeutral);
	chEvtBroadcastI(&ReadyManual);

	chSysUnlockFromIsr();

	if(count >= 15) {
		NeutralStatus.vertNeutral = ~NeutralStatus.vertNeutral;
		NeutralStatus.latNeutral = ~NeutralStatus.vertNeutral;
		count = 0;
	}
	else
	++count;

	return;
}

static GPTConfig gptcfg = {
	.frequency = 100000,
	.callback = ethernet_loop,
	.dier = 0,
};

static void evtSendNeutral(eventid_t id UNUSED){
	BaseSequentialStream *chp = getUsbStream();

	SendNeutral(&NeutralStatus);
	chprintf(chp, "Neutral: Lat %d Vert %d\r\n",
		NeutralStatus.latNeutral, NeutralStatus.vertNeutral);
}

static void evtReceiveManual(eventid_t id UNUSED){
	BaseSequentialStream *chp = getUsbStream();

	ReceiveManual(&ManualStatus);
	chprintf(chp, "Manual: EN %d Mode %d Aux %d lat %d vert %d axis3 %d axis4 %d\r\n",
			ManualStatus.Enable, ManualStatus.Mode, ManualStatus.Aux,
			ManualStatus.latPosition, ManualStatus.vertPosition,
			ManualStatus.Axis3Position, ManualStatus.Axis4Position);
}


void main(void) {
	/* System initializations */
	halInit();
	chSysInit();

	/* Start diagnostics led */
	ledStart(NULL);

	/* Start diagnostics shell */
	const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
	};
	usbSerialShellStart(commands);
	BaseSequentialStream * chp = getUsbStream();

	// Enable Continuous GPT for 500ms Interval
	gptStart(&GPTD1, &gptcfg);
	gptStartContinuous(&GPTD1, 2000000);

	/* Start the lwip thread*/
//	chprintf(chp, "LWIP ");
	lwipThreadStart(RTX_LWIP);

	//Create sockets
//	ReceiveRTxfromSLASocket();
	SendRTxtoManualSocket();
	ReceiveRTxfromManualSocket();


	NeutralStatus.latNeutral = 0;
	NeutralStatus.vertNeutral = 0;


	// Set up event system
	struct EventListener evtNeutral, evtManual, evtSLA;
	chEvtRegister(&ReadyNeutral, &evtNeutral, 0);
	chEvtRegister(&ReadyManual, &evtManual, 1);
	//chEvtRegister(&ReadySLA, &evtSLA, 2);
	const evhandler_t evhndl[] = {
		evtSendNeutral,
		evtReceiveManual,
//		evtReceiveSLA,
	};


	while (TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
