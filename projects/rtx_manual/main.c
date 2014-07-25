/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include "rocket_tracks.h"

#define ADC_ACCUM_WT    90
#define ADC_SAMPLE_WT   10
#define ADC_ACCUM_DIV   (ADC_ACCUM_WT + ADC_SAMPLE_WT)

/* Total number of feedback channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   1
/* Depth of the conversion buffer, channels are sampled once each.*/
#define ADC_GRP1_BUF_DEPTH      1

#define GPIOA_ENABLE            6
#define GPIOE_MODE              4
#define GPIOE_AUX               6
#define GPIOG_MODE1LED          12
#define GPIOG_MODE2LED          15
#define GPIOC_AUX1LED           0
#define GPIOF_AUX2LED           8
#define GPIOF_LATNEUTLED        10
#define GPIOF_VERTNEUTLED       9
#define GPIOA_LATINPUT          4
#define GPIOA_VERTINPUT         5

//Function Prototypes
static void convert_start(GPTDriver *gptp);
static void adccb1(ADCDriver *adcp, adcsample_t *buffer, size_t n);
static void adccb2(ADCDriver *adcp, adcsample_t *buffer, size_t n);

/*
 * ADC conversion group 1.
 * Mode:
 * Channels:    IN4.
 */
static const ADCConversionGroup adcgrpcfg1 = {
	FALSE,
	ADC_GRP1_NUM_CHANNELS,
	adccb1,
	NULL,
	/* HW dependent part.*/
	0,
	ADC_CR2_SWSTART,
	0,
	ADC_SMPR2_SMP_AN4(ADC_SAMPLE_56),
	ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
	0,
	ADC_SQR3_SQ1_N(ADC_CHANNEL_IN4),
};

static const ADCConversionGroup adcgrpcfg2 = {
	FALSE,
	ADC_GRP1_NUM_CHANNELS,
	adccb2,
	NULL,
	/* HW dependent part.*/
	0,
	ADC_CR2_SWSTART,
	0,
	ADC_SMPR2_SMP_AN5(ADC_SAMPLE_56),
	ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
	0,
	ADC_SQR3_SQ1_N(ADC_CHANNEL_IN5)
};



// Global variables

// Shell Override Variables
uint8_t U8ShellEnable = ENABLED;
uint8_t U8ShellMode = NO_SHELL_MODE;
uint8_t latAxisFreeze = DISABLED;
uint8_t vertAxisFreeze = DISABLED;

// Axis Input Variables
static adcsample_t Samples[AXIS_ADC_COUNT];
static uint32_t AxisAccumulator[AXIS_ADC_COUNT];
static uint8_t vertFreeze;
static uint8_t latFreeze;

// Message structs
ManualData ManualInputs;
Neutral NeutralFeedback;
Diagnostics latDiagnostics, vertDiagnostics;
uint16_t refVoltage;

static EVENTSOURCE_DECL(ReadyManual);


/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

/******************************************************************************
 * Function name:       DisplayData
 *
 * Description:         Displays axis data
 *
 *****************************************************************************/
void DisplayData(BaseSequentialStream *chp, Diagnostics *axis_p) {

	chprintf(chp, "   FeedbackADC: %d counts \r\n", axis_p->U16FeedbackADC);
	chprintf(chp, "   PreviousADC: %d counts \r\n", axis_p->U16FeedbackADCPrevious);

	chprintf(chp, "Output Command: %d \r\n", axis_p->S16OutputCommand);

	chprintf(chp, "PositionActual: %d counts \r\n", axis_p->U16PositionActual);
	chprintf(chp, "       Desired: %d counts\r\n", axis_p->U16PositionDesired);
	chprintf(chp, " PositionError: %d counts\r\n", axis_p->S16PositionError);

	chprintf(chp, "        P Term: %d counts\r\n", axis_p->S32PositionPTerm);
	chprintf(chp, "        I Term: %d counts\r\n", axis_p->S32PositionITerm);
	chprintf(chp, "        D Term: %d counts\r\n", axis_p->S32PositionDTerm);
	chprintf(chp, "       I Accum: %d counts\r\n", axis_p->S32PositionDTerm);

}

/******************************************************************************
 * Function name:       cmd_data
 *
 * Description:         Displays control data
 *
 * Arguments:           NULL: Displays data for both axes
 *                      l: Displays data for lateral axis
 *                      v: Displays data for vertical axis
 *
 *****************************************************************************/
static void cmd_data(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv; (void)argc;

	if(U8ShellMode != NO_SHELL_MODE) {
		chprintf(chp, "\tShell Override ");
		if(U8ShellMode == SIGHTLINE_MODE) {
			chprintf(chp, "Mode : Sightline\r\n");
		}
		else {
			chprintf(chp, "Mode : Manual\r\n");
		}
	}
	else {
		if(ManualInputs.Mode == SIGHTLINE_MODE) {
			chprintf(chp, "Mode : Sightline\r\n");
		}
		else {
			chprintf(chp, "Mode : Manual\r\n");
		}
	}
	chprintf(chp, "\nMan latPosition: %d\r\n", ManualInputs.latPosition);
	chprintf(chp, "Man vertPosition: %d\r\n", ManualInputs.vertPosition);

	chprintf(chp, "\nReference V: %d\r\n", refVoltage);

	// Lateral axis data
	if(argc == 0 || *argv[0] != 'v') {
	chprintf(chp, "\nLateral Axis : \r\n");
	DisplayData(chp, &latDiagnostics);
	}
	// Vertical axis data
	if(argc == 0 || *argv[0] != 'l') {
	chprintf(chp, "\nVertical Axis : \r\n");
	DisplayData(chp, &vertDiagnostics);
	}
}

/******************************************************************************
 * Function name:       cmd_stop
 *
 * Description:         Disables axes
 *
 * Arguments:           NULL: Disables all axes
 *
 *****************************************************************************/
static void cmd_stop(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: stop\r\n");
	}
	U8ShellEnable = DISABLED;
	chprintf(chp, "System Disabled. Type 'enable' to resume. \r\n");
}

/******************************************************************************
 * Function name:       cmd_enable
 *
 * Description:         Enables axes
 *
 * Arguments:           NULL: Enables all axes
 *
 *****************************************************************************/
static void cmd_enable(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: Enable\r\n");
		return;
	}
	U8ShellEnable = ENABLED;
	chprintf(chp, "System Enabled.\r\n");
}

/******************************************************************************
 * Function name:       cmd_freeze
 *
 * Description:         Switches to position mode and holds axes in current
 *                      commanded position
 *
 * Arguments:           NULL: Freezes all axes
 *                      l: Freezes lateral axis
 *                      v: Freezes vertical axis
 *
 *****************************************************************************/
static void cmd_freeze(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: Freeze\r\n");
		return;
	}
	if (*argv[0] == 'V' || *argv[0] == 'v') {
		U8ShellMode = FREEZE_MODE;
		vertFreeze = ENABLED;
		chprintf(chp, "Vertical Axis ");
	}
	else if(*argv[0] == 'L' || *argv[0] == 'l') {
		U8ShellMode = FREEZE_MODE;
		latFreeze = ENABLED;
		chprintf(chp, "Lateral Axis ");
	}
	else {
		U8ShellMode = FREEZE_MODE;
		latFreeze = ENABLED;
		vertFreeze = ENABLED;
		chprintf(chp, "Both Axes ");
	}
	chprintf(chp, "Frozen.\r\n");
}

/******************************************************************************
 * Function name:       cmd_unfreeze
 *
 * Description:         Switches to position mode and holds axes in current
 *                      commanded position
 *
 * Arguments:           NULL: Unfreezes all axes
 *                      l: Unfreezes lateral axis
 *                      v: Unfreezes vertical axis
 *
 *****************************************************************************/
static void cmd_unfreeze(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: Unfreeze\r\n");
		return;
	}
	if (*argv[0] == 'V' || *argv[0] == 'v') {
		U8ShellMode = NO_SHELL_MODE;
		vertFreeze = DISABLED;
		chprintf(chp, "Vertical Axis ");
	}
	else if(*argv[0] == 'L' || *argv[0] == 'l') {
		U8ShellMode = NO_SHELL_MODE;
		latFreeze = DISABLED;
		chprintf(chp, "Lateral Axis ");
	}
	else {
		U8ShellMode = NO_SHELL_MODE;
		latFreeze = DISABLED;
		vertFreeze = DISABLED;
		chprintf(chp, "Both Axes ");
	}
	chprintf(chp, "Unfrozen.\r\n");

	if(ManualInputs.Mode == MANUAL_MODE) {
		chprintf(chp, "Mode : Manual\r\n");
	}
	else {
		chprintf(chp, "Mode : Sightline\r\n");
	}
}

/******************************************************************************
 * Function name:       cmd_mode
 *
 * Description:         Selects/displays mode or reverts to mode switch state
 *
 * Arguments:           NULL: Displays current mode
 *                      MANUAL_MODE: Shell Override Mode: Manual
 *                      SIGHTLINE_MODE: Shell Override Mode: Sightline
 *                      NO_SHELL_MODE: Mode switch
 *
 *****************************************************************************/
static void cmd_mode(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		if (*argv[0] == '0')
			U8ShellMode = MANUAL_MODE;
		else if(*argv[0] == '1')
			U8ShellMode = SIGHTLINE_MODE;
		else if(*argv[0] == '2')
			U8ShellMode = NO_SHELL_MODE;
	}
	if(U8ShellMode != NO_SHELL_MODE) {
		chprintf(chp, "\tShell Override ");
		if(U8ShellMode == SIGHTLINE_MODE) {
			chprintf(chp, "Mode : Sightline\r\n");
		}
		else {
			chprintf(chp, "Mode : Manual\r\n");
		}
	}
	else {
		if(ManualInputs.Mode == SIGHTLINE_MODE) {
			chprintf(chp, "Mode : Sightline\r\n");
		}
		else {
			chprintf(chp, "Mode : Manual\r\n");
		}
	}
}

/*
 * ADC1 end conversion callback.
 * The lateral axis control loop is run and PWM updated.
 */
static void adccb1(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

	(void) buffer; (void) n;
	/* Note, only in the ADC_COMPLETE state because the ADC driver fires an
	 intermediate callback when the buffer is half full.*/
	if (adcp->state == ADC_COMPLETE) {
		// Weighted Moving Average accumulator of axis input samples produces 15-bit value
		AxisAccumulator[LAT_AXIS] = ((AxisAccumulator[LAT_AXIS] * ADC_ACCUM_WT)
				+ (Samples[LAT_AXIS] * ADC_SAMPLE_WT))/ADC_ACCUM_DIV;
	}
}

/*
 * ADC2 end conversion callback.
 * The vertical axis control loop is run and PWM updated.
 */
static void adccb2(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

	(void) buffer; (void) n;
	/* Note, only in the ADC_COMPLETE state because the ADC driver fires an
	 intermediate callback when the buffer is half full.*/
	if (adcp->state == ADC_COMPLETE) {
		// Weighted Moving Average accumulator of axis input samples produces 15-bit value
		AxisAccumulator[VERT_AXIS] = ((AxisAccumulator[VERT_AXIS] * ADC_ACCUM_WT)
				+ (Samples[VERT_AXIS] * ADC_SAMPLE_WT))/ADC_ACCUM_DIV;
	}
}

//static void evtReceiveNeutral(eventid_t id UNUSED){
//
//    ReceiveNeutral(&NeutralFeedback);
//}

static void evtSendManual(eventid_t id UNUSED){

	SendManual(&ManualInputs);
}
static void evtProcessLEDs(eventid_t id UNUSED){

	//Update Manual Control Box LEDs
	// Set Neutral LEDs based on axis Neutral state
	if(NeutralFeedback.vertNeutral == DISABLED) {
		palSetPad(GPIOF, GPIOF_VERTNEUTLED);
	}
	else {
		palClearPad(GPIOF, GPIOF_VERTNEUTLED);
	}
	if(NeutralFeedback.latNeutral == DISABLED) {
		palSetPad(GPIOF, GPIOF_LATNEUTLED);
	}
	else {
		palClearPad(GPIOF, GPIOF_LATNEUTLED);
	}

	// Set Mode LEDs based on Mode Switch state
	if(ManualInputs.Mode == SIGHTLINE_MODE) {
		palClearPad(GPIOG, GPIOG_MODE1LED);
		palSetPad(GPIOG, GPIOG_MODE2LED);
	}
	else if(ManualInputs.Mode == MANUAL_MODE) {
		palSetPad(GPIOG, GPIOG_MODE1LED);
		palClearPad(GPIOG, GPIOG_MODE2LED);
	}

	return;
}

/*
 * Manual Control Loop.
 */
static void manual_loop(GPTDriver *gptp) {

	(void) gptp;

	chSysLockFromIsr();

	// Read Enable switch state
	if(U8ShellEnable) {
		ManualInputs.Enable = palReadPad(GPIOA, GPIOA_ENABLE); // PD8
	}
	else {
		ManualInputs.Enable = DISABLED;
	}
	// Read Mode Switch
	ManualInputs.Mode = palReadPad(GPIOE, GPIOE_MODE); // PD9

	if(ManualInputs.Enable) {

		//Send Sightline Mode message to RTx if Enable and Sightline Mode
		if(ManualInputs.Mode == SIGHTLINE_MODE) {
			// Send Message to RTx
			chEvtBroadcastI(&ReadyManual);
		}
		else {

			// Set desired position values to send to RTx Controller.
			// If axis frozen, don't update desired positions.
			if(latFreeze == DISABLED)
				ManualInputs.latPosition = (AxisAccumulator[LAT_AXIS]) * 16;
			if(vertFreeze == DISABLED)
				ManualInputs.vertPosition = (AxisAccumulator[VERT_AXIS]) * 16;

			// Send message to RTx Controller
			chEvtBroadcastI(&ReadyManual);
		}
	}
	else {
		// Send disable message to RTx
		chEvtBroadcastI(&ReadyManual);
	}


	chSysUnlockFromIsr();

	//Update Manual Control Box LEDs
	evtProcessLEDs(0);
	return;
}

static const GPTConfig gpt1cfg = {
	1000000,
	manual_loop,
	0,
};

/*
 * Starts new conversion of axis input ADCs.
 */
static void convert_start(GPTDriver *gptp) {

	(void) gptp;

	if(ManualInputs.Enable && ManualInputs.Mode == MANUAL_MODE) {

		//Read Input ADCs
		chSysLockFromIsr();
		//Read lat input ADC
		adcStartConversionI(&ADCD1, &adcgrpcfg1, &Samples[LAT_AXIS], 1);
		//Read vert input ADC
		adcStartConversionI(&ADCD2, &adcgrpcfg2, &Samples[VERT_AXIS], 1);
		chSysUnlockFromIsr();

	return;

	}
}

static const GPTConfig gpt2cfg = {
	1000000,
	convert_start,
	0,
};

WORKING_AREA(wa_rx, 512);
msg_t rx_thread(void *p UNUSED) {
	chRegSetThreadName("rx");
	/*
	 * This thread creates a UDP socket and then listens for any incoming
	 * message, printing it out over serial USB
	 */



	//read data from socket
	while(TRUE) {
		ReceiveNeutral(&NeutralFeedback);
	}

	return -1;
}

WORKING_AREA(wa_diagrx, 512);
msg_t diagrx_thread(void *p UNUSED) {
	chRegSetThreadName("rx");
	/*
	 * This thread creates a UDP socket and then listens for any incoming
	 * message, printing it out over serial USB
	 */



	//read data from socket
	while(TRUE) {
		ReceiveDiagnostics(&latDiagnostics, &vertDiagnostics, &refVoltage);
	}

	return -1;
}

/*
 * Application entry point.
 */
int main(void) {

	/*
	* System initializations.
	* - HAL initialization, this also initializes the configured device drivers
	*   and performs the board-specific initializations.
	* - Kernel initialization, the main() function becomes a thread and the
	*   RTOS is active.
	*/

	halInit();
	chSysInit();


	/* Start diagnostics led */
	ledStart(NULL);


	/* Start diagnostics shell */
	const ShellCommand commands[] = {
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{"data", cmd_data},
		{"stop", cmd_stop},
		{"disable", cmd_stop},
		{"enable", cmd_enable},
		{"mode", cmd_mode},
		{"freeze", cmd_freeze},
		{"unfreeze", cmd_unfreeze},
		{NULL, cmd_stop}
	};
	usbSerialShellStart(commands);
	BaseSequentialStream * chp = getUsbStream();

	// Enable Continuous GPT for 10ms Interval for control loop
	gptStart(&GPTD1, &gpt1cfg);
	gptStartContinuous(&GPTD1, 9200);

	// Enable Continuous GPT for 500us Interval for ADC conversions
	gptStart(&GPTD2, &gpt2cfg);
	gptStartContinuous(&GPTD2, 500);

	/* Start the lwip thread*/
	chprintf(chp, "LWIP ");
	lwipThreadStart(RTXMAN_LWIP);

	//Create sockets
	SendManualtoRTxSocket();
	ReceiveManualfromRTxSocket();
	ReceiveDiagnosticsSocket();

	chThdCreateStatic(wa_rx, sizeof(wa_rx), NORMALPRIO, rx_thread, NULL);
	chThdCreateStatic(wa_diagrx, sizeof(wa_diagrx), NORMALPRIO, diagrx_thread, NULL);

	// Configure pins for Input ADC's
	// PA4: Lat Axis Input
	palSetPadMode(GPIOA, GPIOA_LATINPUT, PAL_MODE_INPUT_ANALOG);
	// PA5: Vert Axis Input
	palSetPadMode(GPIOA, GPIOA_VERTINPUT, PAL_MODE_INPUT_ANALOG);

	// Configure pins for Mode LEDs
	// PG12: Mode1 LED
	palSetPadMode(GPIOG, GPIOG_MODE1LED, PAL_MODE_OUTPUT_PUSHPULL);
	// PG15: Mode2 LED
	palSetPadMode(GPIOG, GPIOG_MODE2LED, PAL_MODE_OUTPUT_PUSHPULL);

	// Configure pins for Aux LEDs
	// PC0: Aux1 LED
	palSetPadMode(GPIOC, GPIOC_AUX1LED, PAL_MODE_OUTPUT_PUSHPULL);
	// PF8: Aux2 LED
	palSetPadMode(GPIOF, GPIOF_AUX2LED, PAL_MODE_OUTPUT_PUSHPULL);

	// Configure pins for Neutral LEDs
	// PF10: lat Neutral LED
	palSetPadMode(GPIOF, GPIOF_LATNEUTLED, PAL_MODE_OUTPUT_PUSHPULL);
	// PF9: vert Neutral LED
	palSetPadMode(GPIOF, GPIOF_VERTNEUTLED, PAL_MODE_OUTPUT_PUSHPULL);

	// Configure pins for switches
	// PA6: Drive Enable Switch
	palSetPadMode(GPIOA, GPIOA_ENABLE, PAL_MODE_INPUT_PULLDOWN);
	// PE4: Mode Select Switch
	palSetPadMode(GPIOE, GPIOE_MODE, PAL_MODE_INPUT);
	// PE6: Aux Select Switch
	palSetPadMode(GPIOE, GPIOE_AUX, PAL_MODE_INPUT);

	adcStart(&ADCD1, NULL);
	adcStart(&ADCD2, NULL);

	NeutralFeedback.latNeutral = DISABLED;
	NeutralFeedback.vertNeutral = DISABLED;

	// Set up event system
	struct EventListener evtManual;
	chEvtRegister(&ReadyManual, &evtManual, 0);

	const evhandler_t evhndl[] = {
		evtSendManual,
	};


	while (TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
