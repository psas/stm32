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

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "lwipthread.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "utils_sockets.h"
#include "enet_api.h"
#include "net_addrs.h"

#include "utils_general.h"
#include "utils_led.h"

#include "control.h"
#include "rtx_utils.h"
#include "rocket_tracks.h"

ManualData ManualStatus;
Neutral NeutralStatus;
SLAData SLAStatus;
Diagnostics latDiagnostics, vertDiagnostics;

static EVENTSOURCE_DECL(ReadyNeutral);
static EVENTSOURCE_DECL(ReadyDiagnostics);

#define ADC_ACCUM_WT    85
#define ADC_SAMPLE_WT   15
#define ADC_ACCUM_DIV   (ADC_ACCUM_WT + ADC_SAMPLE_WT)

#define NEUTRAL_THRESH  512

/* Total number of feedback channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   1
/* Depth of the conversion buffer, channels are sampled once each.*/
#define ADC_GRP1_BUF_DEPTH      1

//Function Prototypes
static void watchdog(void);
static void enable_axes(uint8_t enable);
static void ControlAxis(CONTROL_AXIS_STRUCT *ptr, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN);
static void spicb(SPIDriver *spip);
static void convert_start(GPTDriver *gptp);
static void motordrive(GPTDriver *gptp);
static uint8_t DriveEnable (void);

/*
 * PWM configuration structure.
 */
PWMConfig pwmcfg = {
	1000000, /* 1Mhz PWM clock frequency */
	1000,   /* PWM period 1 millisecond */
	NULL,    /* No callback */
	/* All 4 channels enabled */
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0,
	0
};

/*
 * SPI1 configuration structure.
 */
static const SPIConfig spi1cfg = {
	spicb,
	/* HW dependent part.*/
	GPIOA,
	GPIOA_CS_SPI,
	SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_DFF | SPI_CR1_BR_0 | SPI_CR1_CPOL | SPI_CR1_CPHA,
};

static const ADCConversionGroup adcgrpcfg1 = {
	FALSE,
	ADC_GRP1_NUM_CHANNELS,
	NULL,
	NULL,
	/* HW dependent part.*/
	0,
	ADC_CR2_SWSTART,
	0,
	ADC_SMPR2_SMP_AN5(ADC_SAMPLE_56),
	ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
	0,
	ADC_SQR3_SQ1_N(ADC_CHANNEL_IN5),
};

// Global variables
static int count = 0;

// Shell Override Variables
uint8_t U8ShellEnable = ENABLED;

// ADC Sample Variables
static axissample_t Samples[AXIS_ADC_COUNT];
static uint32_t AxisAccumulator[AXIS_ADC_COUNT];
static adcsample_t refMonitor;

// Axis Control Variables
uint8_t U8PrevPosnVelModeSwitchState = DISABLED;
uint32_t u32Temp = 0;
CONTROL_AXIS_STRUCT vertAxisStruct, latAxisStruct;
static uint8_t u8WatchDogCycleCount = 0;
static uint8_t latOutofrangehigh = 0;
static uint8_t latOutofrangelow = 0;
static uint8_t vertOutofrangehigh = 0;
static uint8_t vertOutofrangelow = 0;
static volatile uint16_t ManualWatchdog = 0;

/*
 * SPI end transfer callback.
 */
static void spicb(SPIDriver *spip) {

	(void)spip;

	/* On transfer end deasserts CONV pin and accumulates results.*/
	palClearPad(GPIOA, GPIOA_ADC_CNV);

	// Weighted Moving Average accumulator of axis feedback samples
	AxisAccumulator[LAT_AXIS] = ((AxisAccumulator[LAT_AXIS] * ADC_ACCUM_WT) + (Samples[LAT_AXIS] * ADC_SAMPLE_WT))/ADC_ACCUM_DIV;
	AxisAccumulator[VERT_AXIS] = ((AxisAccumulator[VERT_AXIS] * ADC_ACCUM_WT) + (Samples[VERT_AXIS] * ADC_SAMPLE_WT))/ADC_ACCUM_DIV;
}

/*
 * Starts new conversion of axis input ADCs with polling.
 */
static void convert_start(GPTDriver *gptp) {

	(void) gptp;

	//Read Input ADCs
	palSetPad(GPIOA, GPIOA_ADC_CNV);

	// Poll MISO, wait for CDONE signal
	while(palReadPad(GPIOA, GPIOA_SPI1_MISO) == 0);

	chSysLockFromIsr();
	spiStartReceiveI(&SPID1, AXIS_ADC_COUNT, (uint16_t *)Samples);
	chSysUnlockFromIsr();

	return;
}

static const GPTConfig gpt2cfg = {
	1000000,
	convert_start,
	0,
};

static void evtSendNeutral(eventid_t id UNUSED){
	SendNeutral(&NeutralStatus);
}

static void evtSendDiagnostics(eventid_t id UNUSED){
	SendDiagnostics(&latDiagnostics, &vertDiagnostics, refMonitor);
}

WORKING_AREA(wa_rx, 512);
msg_t rx_thread(void *p UNUSED) {
	chRegSetThreadName("rx");
	/*
	 * This thread creates a UDP socket and then listens for any incoming
	 * message, printing it out over serial USB
	 */



	//read data from socket
	while(TRUE) {
		if(ReceiveManual(&ManualStatus) > 0) {
			ManualWatchdog = 0;
		}
	}

    return -1;
}

WORKING_AREA(wa_slarx, 512);
msg_t slarx_thread(void *p UNUSED) {
	chRegSetThreadName("rx");
	/*
	 * This thread creates a UDP socket and then listens for any incoming
	 * message, printing it out over serial USB
	 */



	//read data from socket
	while(TRUE) {
		if(ReceiveSLA(&SLAStatus) > 0) {
			if(ManualStatus.Mode == SIGHTLINE_MODE) {
				Process_SLA(&SLAStatus, &latAxisStruct, &vertAxisStruct);
			}
		}
	}

    return -1;
}

static void SetDiagnostics(Diagnostics * data, CONTROL_AXIS_STRUCT * axis) {

	data->U16FeedbackADC = axis->U16FeedbackADC;
	data->U16FeedbackADCPrevious = axis->U16FeedbackADCPrevious;
	data->S16OutputCommand = axis->S16OutputCommand;
	data->U16PositionDesired = axis->U16PositionDesired;
	data->U16PositionActual = axis->U16PositionActual;
	data->S16PositionError = axis->S16PositionError;
	data->S16PositionErrorPrevious = axis->S16PositionErrorPrevious;
	data->S32PositionPTerm = axis->S32PositionPTerm;
	data->S32PositionITerm = axis->S32PositionITerm;
	data->S32PositionDTerm = axis->S32PositionDTerm;
	data->S32PositionIAccumulator = axis->S32PositionIAccumulator;
}

/*
 * GMD control thread, times are in microseconds.
 */
static void motordrive(GPTDriver *gptp) {

	(void) gptp;

	chSysLockFromIsr();

	palTogglePad(GPIOE, GPIOE_PIN3);
	// Read Feedback ADC's
	latAxisStruct.U16FeedbackADC = (axissample_t)AxisAccumulator[LAT_AXIS];
	vertAxisStruct.U16FeedbackADC = (axissample_t)AxisAccumulator[VERT_AXIS];

	if(ManualStatus.Mode == MANUAL_MODE) {
		// Set desired positions based on Manual Remote inputs
		latAxisStruct.U16PositionDesired = ManualStatus.latPosition;
		vertAxisStruct.U16PositionDesired = ManualStatus.vertPosition;
	}

	if(count >= 75) {
		// Send Neutral and Diagnostic Status to Manual Control Box
		SetDiagnostics(&latDiagnostics, &latAxisStruct);
		SetDiagnostics(&vertDiagnostics, &vertAxisStruct);
		chEvtBroadcastI(&ReadyNeutral);
		chEvtBroadcastI(&ReadyDiagnostics);
		count = 0;
	}
	else
		++count;


	chSysUnlockFromIsr();


	// Drive Enable Safety Interlock Section
	// DANGER - Safety-Critical Code Section

	DriveEnable();

	//Run Control Loops
	ControlAxis(&latAxisStruct, 4, 3);
	ControlAxis(&vertAxisStruct, 2, 1);

	// END - Safety-Critical Code Section

	return;
}

static const GPTConfig gpt1cfg = {
	1000000,
	motordrive,
	0,
};


/******************************************************************************
 * Function name:       ControlAxis
 *
 * Description:         Updates PWM outputs based on system state
 *****************************************************************************/
void ControlAxis(CONTROL_AXIS_STRUCT *axis_p, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN) {

	uint32_t useconds;

	chSysLockFromIsr();

	// Run control loop
	controlLoop(axis_p);

	// Set axis PWM's
	if(axis_p->S16OutputCommand > 0){ //Forward
		// Compute ON time scaled 0 - 511 counts
		// Can't go to 100% DC, so max is 98%, or 958
		useconds = ((uint32_t)(axis_p->S16OutputCommand * (2 * 958))) / 1000;
		pwmEnableChannelI(&PWMD4, PWM_U_CHAN-1, usecondsToPWMTicks(useconds, pwmcfg.frequency));
		pwmEnableChannelI(&PWMD4, PWM_V_CHAN-1, usecondsToPWMTicks(0, pwmcfg.frequency));
	}else{ // Reverse
		// Compute ON time scaled 0 - 511 counts
		// Can't go to 100% DC, so max is 98%, or 958
		useconds = ((uint32_t)(axis_p->S16OutputCommand * (-2 * 958))) / 1000;
		pwmEnableChannelI(&PWMD4, PWM_U_CHAN-1, usecondsToPWMTicks(0, pwmcfg.frequency));
		pwmEnableChannelI(&PWMD4, PWM_V_CHAN-1, usecondsToPWMTicks(useconds, pwmcfg.frequency));
	}
	chSysUnlockFromIsr();
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

	//Start SPI1 peripheral
	spiStart(&SPID1, &spi1cfg);

	adcStart(&ADCD1, NULL);

	// Configure pins for PWM output (D12-D15: TIM4, channel 1-4)
	palSetPadMode(GPIOD, GPIOD_VERT_V, PAL_MODE_ALTERNATE(2));	//U-pole, short lead
	palSetPadMode(GPIOD, GPIOD_VERT_U, PAL_MODE_ALTERNATE(2));	//V-pole, short lead
	palSetPadMode(GPIOD, GPIOD_LAT_V, PAL_MODE_ALTERNATE(2));	//U-pole, long lead
	palSetPadMode(GPIOD, GPIOD_LAT_U, PAL_MODE_ALTERNATE(2));	//V-pole, long lead

	palClearPad(GPIOA, GPIOA_ADC_CNV);

	pwmStart(&PWMD4, &pwmcfg);


	// Set axis control gain and limit values
	// Set Vertical Axis Gains
	vertAxisStruct.U16PositionPGain = 30;
	vertAxisStruct.U16PositionIGain = 0;
	vertAxisStruct.U16PositionDGain = 0;
	// Set vertical axis limits
	vertAxisStruct.U16CommandLimit = VERTICAL_COMMAND_LIMIT;
	vertAxisStruct.U16HighPosnLimit = VERT_HIGH_POS_MAX;
	vertAxisStruct.U16LowPosnLimit = VERT_LOW_POS_MIN;
	// Set Lateral Axis Gains
	latAxisStruct.U16PositionPGain = 15;
	latAxisStruct.U16PositionIGain = 0;
	latAxisStruct.U16PositionDGain = 0;
	// Set lateral axis limits
	latAxisStruct.U16CommandLimit = LATERAL_COMMAND_LIMIT;
	latAxisStruct.U16HighPosnLimit = LAT_HIGH_POS_MAX;
	latAxisStruct.U16LowPosnLimit = LAT_LOW_POS_MIN;

	// Enable Continuous GPT for 10ms Interval
	gptStart(&GPTD1, &gpt1cfg);
	gptStartContinuous(&GPTD1,10000);

	// Enable Continuous GPT for 500us Interval for ADC conversions
	gptStart(&GPTD2, &gpt2cfg);
	gptStartContinuous(&GPTD2, 500);

	/* Start the lwip thread*/
	lwipThreadStart(RTX_LWIP);

	//Create sockets
	ReceiveRTxfromSLASocket();
	SendRTxtoManualSocket();
	SendDiagnosticsSocket();
	ReceiveRTxfromManualSocket();

	chThdCreateStatic(wa_rx, sizeof(wa_rx), NORMALPRIO, rx_thread, NULL);
	chThdCreateStatic(wa_slarx, sizeof(wa_slarx), NORMALPRIO, slarx_thread, NULL);

	// Set up event system
	struct EventListener evtNeutral, evtDiagnostics;
	chEvtRegister(&ReadyNeutral, &evtNeutral, 0);
	chEvtRegister(&ReadyDiagnostics, &evtDiagnostics, 1);
	const evhandler_t evhndl[] = {
		evtSendNeutral,
		evtSendDiagnostics,
	};

	while (TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}

/******************************************************************************
 * Function name:       watchdog
 *
 * Description:         Toggles the Watchdog outputs to the GMD's
 *
 *****************************************************************************/
static void watchdog() {

	// Handle HW Watchdog on GMD
	if(u8WatchDogCycleCount == 1){
		palTogglePad(GPIOD, GPIOD_LAT_WD);
		palTogglePad(GPIOD, GPIOD_VERT_WD);
		u8WatchDogCycleCount = 0;
	}
	else
		u8WatchDogCycleCount++;
}

/******************************************************************************
 * Function name:       enable_axes
 *
 * Description:         Asserts or deasserts all axis Enable outputs to the GMD's
 *                      based on the value of enable.
 *
 *****************************************************************************/
static void enable_axes(uint8_t enable) {

	// Enable all axes
	if(enable == ENABLED) {
		palSetPad(GPIOD, GPIOD_LAT_EN);
		palSetPad(GPIOD, GPIOD_VERT_EN);
	}
	// Disable all axes
	else {
		palClearPad(GPIOD, GPIOD_LAT_EN);
		palClearPad(GPIOD, GPIOD_VERT_EN);
	}
}

/******************************************************************************
 * Function name:       DriveEnable
 *
 * Description:         DANGER - LIFE-SAFETY CRITICAL CODE - THIS CODE WAS
 *                      DEVELOPED BASED ON RESULTS OF SYSTEM FMEA AND SHOULD BE
 *                      MODIFIED ONLY BY QUALIFIED PERSONNEL.
 *
 *                      Performs safety interlock checks before enabling GMDs and
 *                      toggling watchdog as appropriate.
 *
 *                      Returns ENABLED for Drives Enabled
 *                      Returns DISABLED for Drives Disabled
 *****************************************************************************/
static uint8_t DriveEnable(void) {

	uint8_t UserEnable = DISABLED;
	uint8_t TestEnable = ENABLED;
	uint8_t ApproveEnable = DISABLED;


	// Increment/saturate Manual Remote Watchdog counter
	if(ManualWatchdog <= MANUAL_WATCHDOG_TIMEOUT)
		++ManualWatchdog;

/* DRIVE ENABLE CHECKS SECTION */

	// Check for User Enable Status (Manual Enable Switch and Shell Override)
	if(ManualStatus.Enable == ENABLED && U8ShellEnable == ENABLED) {
		UserEnable = ENABLED;
	}
	else {
		UserEnable = DISABLED;
		latOutofrangehigh = 0;
		latOutofrangelow = 0;
		vertOutofrangehigh = 0;
		vertOutofrangelow = 0;
	}

//	// Check for Reference Voltage OK
//	if(refMonitor < REFMONITOR_THRESH)
//		TestEnable = DISABLED;


	// Check for out-of-range lat axis positions
	if(Samples[LAT_AXIS] >= LAT_HIGH_MAX) {
		++latOutofrangehigh;
		latOutofrangelow = 0;
	}
	else if(Samples[LAT_AXIS] <= LAT_LOW_MIN) {
		++latOutofrangelow;
		latOutofrangehigh = 0;
	}
	else {
		latOutofrangehigh = 0;
		latOutofrangelow = 0;
	}
	if(vertOutofrangelow > OUT_OF_RANGE_TIMEOUT ||
			vertOutofrangehigh > OUT_OF_RANGE_TIMEOUT) {
		TestEnable = DISABLED;
	}

	// Check for out-of-range vert axis positions
	if(Samples[VERT_AXIS] >= VERT_HIGH_MAX) {
		++vertOutofrangehigh;
		vertOutofrangelow = 0;
	}
	else if(Samples[VERT_AXIS] <= VERT_LOW_MIN) {
		++vertOutofrangelow;
		vertOutofrangehigh = 0;
	}
	else {
		vertOutofrangehigh = 0;
		vertOutofrangelow = 0;
	}
	if(latOutofrangelow > OUT_OF_RANGE_TIMEOUT ||
			latOutofrangehigh > OUT_OF_RANGE_TIMEOUT) {
		TestEnable = DISABLED;

	}

	// Check ManualWatchdog Time-out
	if(ManualWatchdog >= MANUAL_WATCHDOG_TIMEOUT) {
		TestEnable = DISABLED;
	}


	// Check all enable flags and approve Enable
	if(UserEnable && TestEnable)
		ApproveEnable = ENABLED;
	else
		ApproveEnable = DISABLED;


	// Handle ENABLE on GMD and Drive Interlocks on Mode Change
	if(ApproveEnable == ENABLED) {

		// Turn ON enable
		enable_axes(ENABLED);
		// Turn off Enabled LED
		palClearPad(GPIOE, GPIOE_PIN3);
		// Run watchdog
		watchdog();

		// Interlock drives on mode change
		if(ManualStatus.Mode != U8PrevPosnVelModeSwitchState) {
			vertAxisStruct.U8DriveIsInterlocked = ENABLED;
			latAxisStruct.U8DriveIsInterlocked = ENABLED;
		}

	}
	// Disable Drives and Interlock if ENABLE not approved
	else {

		// Otherwise enable is OFF
		enable_axes(DISABLED);
		// Lock out drives
		vertAxisStruct.U8DriveIsInterlocked = ENABLED;
		latAxisStruct.U8DriveIsInterlocked = ENABLED;
		// Turn off Enabled LED
		palSetPad(GPIOE, GPIOE_PIN3);
	}

	// Check for interlocks, unlock if error is low
	if(latAxisStruct.U8DriveIsInterlocked == ENABLED) {
		if(latAxisStruct.U16PositionActual < ManualStatus.latPosition + NEUTRAL_THRESH)
			latAxisStruct.U8DriveIsInterlocked = DISABLED;
		else if(latAxisStruct.U16PositionActual > ManualStatus.latPosition - NEUTRAL_THRESH)
			latAxisStruct.U8DriveIsInterlocked = DISABLED;
	}
	if(vertAxisStruct.U8DriveIsInterlocked == ENABLED) {
		if(vertAxisStruct.U16PositionActual < ManualStatus.vertPosition + NEUTRAL_THRESH)
			vertAxisStruct.U8DriveIsInterlocked = DISABLED;
		else if(vertAxisStruct.U16PositionActual > ManualStatus.vertPosition - NEUTRAL_THRESH)
			vertAxisStruct.U8DriveIsInterlocked = DISABLED;
	}

	// Set Neutral Flags
	NeutralStatus.latNeutral = latAxisStruct.U8DriveIsInterlocked;
	NeutralStatus.vertNeutral = vertAxisStruct.U8DriveIsInterlocked;

	// Record previous Mode to support drive interlock on mode change
	U8PrevPosnVelModeSwitchState = ManualStatus.Mode;

	// Read Reference Voltage Monitor ADC
	chSysLockFromIsr();
	// Read lat input ADC
	adcStartConversionI(&ADCD1, &adcgrpcfg1, &refMonitor, 1);
	chSysUnlockFromIsr();

	return ApproveEnable;
}
