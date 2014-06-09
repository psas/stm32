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
#include "utils_shell.h"
#include "rtx_shell.h"
#include "utils_led.h"

#include "usbdetail.h"

#include "pwmdetail.h"
#include "control.h"
#include "rocket_tracks.h"

ManualData ManualStatus;
Neutral NeutralStatus;
SLAData SLAStatus;
Diagnostics latDiagnostics, vertDiagnostics;

static EVENTSOURCE_DECL(ReadyNeutral);
static EVENTSOURCE_DECL(ReadyDiagnostics);

#define ADC_ACCUM_WT	15
#define ADC_SAMPLE_WT	85
#define ADC_ACCUM_DIV	(ADC_ACCUM_WT + ADC_SAMPLE_WT)

/* Total number of feedback channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   1
/* Depth of the conversion buffer, channels are sampled once each.*/
#define ADC_GRP1_BUF_DEPTH      1
/* Total number of input channels to be sampled by a single ADC operation.*/
#define ADC_GRP2_NUM_CHANNELS   1
/* Depth of the conversion buffer, input channels are sampled once each.*/
#define ADC_GRP2_BUF_DEPTH      1

//Function Prototypes
void DisplayData(BaseSequentialStream *chp, CONTROL_AXIS_STRUCT *axis_p);
static void watchdog(void);
static void enable_axes(uint8_t enable);
static void ControlAxis(CONTROL_AXIS_STRUCT *ptr, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN);
static void spicb(SPIDriver *spip);
static void extReadADCs(EXTDriver *extp, expchannel_t channel);
static void convert_start(GPTDriver *gptp);
static void motordrive(GPTDriver *gptp);
uint32_t microsecondsToPWMTicks(const uint32_t microseconds);
static uint8_t DriveEnable (void);


/*
 * SPI1 configuration structure.
 */
static const SPIConfig spi1cfg = {
  spicb,
  /* HW dependent part.*/
  GPIOA,
  GPIOA_CS_SPI,
//  SPI_CR1_SSI | SPI_CR1_DFF | SPI_CR1_BIDIMODE | SPI_CR1_MSTR | SPI_CR1_SPE | SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_CPHA,
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
uint8_t U8ShellMode = NO_SHELL_MODE;

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
static uint16_t ManualWatchdog = 0;

static const EXTConfig extcfg = {
  {
	{EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_FALLING_EDGE, extReadADCs},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
  }
};


/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

/******************************************************************************
 * Function name:	cmd_data
 *
 * Description:		Displays control data
 *
 * Arguments:		NULL: Displays data for both axes
 * 					l: Displays data for lateral axis
 * 					v: Displays data for vertical axis
 *
 *****************************************************************************/
void cmd_data(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv; (void)argc;

	if(U8ShellMode != NO_SHELL_MODE) {
		chprintf(chp, "\tShell Override ");
		if(U8ShellMode == ENABLED) {
			chprintf(chp, "Mode : Position\r\n");
		}
		else {
			chprintf(chp, "Mode : Velocity\r\n");
		}
	}
	else {
		if(ManualStatus.Mode == SIGHTLINE_MODE) {
			chprintf(chp, "Mode : Position\r\n");
		}
		else {
			chprintf(chp, "Mode : Velocity\r\n");
		}
	}
	// Lateral axis data
	if(argc == 0 || *argv[0] != 'v') {
	chprintf(chp, "Lateral Axis : \r\n");
	DisplayData(chp, &latAxisStruct);
	}
	// Vertical axis data
	if(argc == 0 || *argv[0] != 'l') {
	chprintf(chp, "Vertical Axis : \r\n");
	DisplayData(chp, &vertAxisStruct);
	}
}

/******************************************************************************
 * Function name:	DisplayData
 *
 * Description:		Displays axis data
 *
 *****************************************************************************/
void DisplayData(BaseSequentialStream *chp, CONTROL_AXIS_STRUCT *axis_p) {

	chprintf(chp, " PositionActual : %d counts ", axis_p->U16PositionActual);
	chprintf(chp, "Desired : %d counts\r\n", axis_p->U16PositionDesired);

	chprintf(chp, "VelocityDesired : %d counts\r\n", axis_p->S16VelocityDesired);

	chprintf(chp, "  PositionError : %d counts\r\n", axis_p->S16PositionError);
	chprintf(chp, "    Interlocked : %d\r\n", axis_p->U8DriveIsInterlocked);
	chprintf(chp, "    Pos Neutral : %d\r\n", axis_p->U8PositionNeutral);
	chprintf(chp, "    Vel Neutral : %d\r\n\n", axis_p->U8VelocityNeutral);
}

/******************************************************************************
 * Function name:	cmd_stop
 *
 * Description:		Disables axes
 *
 * Arguments:		NULL: Disables all axes
 *
 *****************************************************************************/
void cmd_stop(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: stop\r\n");
	}
	U8ShellEnable = DISABLED;
	chprintf(chp, "System Disabled. Type 'enable' to resume. \r\n");
}

/******************************************************************************
 * Function name:	cmd_enable
 *
 * Description:		Enables axes
 *
 * Arguments:		NULL: Enables all axes
 *
 *****************************************************************************/
void cmd_enable(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: Enable\r\n");
		return;
	}
	U8ShellEnable = ENABLED;
	chprintf(chp, "System Enabled.\r\n");
}

/******************************************************************************
 * Function name:	cmd_freeze
 *
 * Description:		Switches to position mode and holds axes in current
 * 					commanded position
 *
 * Arguments:		NULL: Freezes all axes
 * 					l: Freezes lateral axis
 * 					v: Freezes vertical axis
 *
 *****************************************************************************/
void cmd_freeze(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: Freeze\r\n");
		return;
	}
	if (*argv[0] == 'V' || *argv[0] == 'v') {
		U8ShellMode = FREEZE_MODE;
		vertAxisStruct.U8FreezeAxis = ENABLED;
		chprintf(chp, "Vertical Axis ");
	}
	else if(*argv[0] == 'L' || *argv[0] == 'l') {
		U8ShellMode = FREEZE_MODE;
		latAxisStruct.U8FreezeAxis = ENABLED;
		chprintf(chp, "Lateral Axis ");
	}
	else {
		U8ShellMode = FREEZE_MODE;
		latAxisStruct.U8FreezeAxis = ENABLED;
		vertAxisStruct.U8FreezeAxis = ENABLED;
		chprintf(chp, "Both Axes ");
	}
	chprintf(chp, "Frozen.\r\n");
}

/******************************************************************************
 * Function name:	cmd_unfreeze
 *
 * Description:		Switches to position mode and holds axes in current
 * 					commanded position
 *
 * Arguments:		NULL: Unfreezes all axes
 * 					l: Unfreezes lateral axis
 * 					v: Unfreezes vertical axis
 *
 *****************************************************************************/
void cmd_unfreeze(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: Unfreeze\r\n");
		return;
	}
	if (*argv[0] == 'V' || *argv[0] == 'v') {
		U8ShellMode = NO_SHELL_MODE;
		vertAxisStruct.U8FreezeAxis = DISABLED;
		chprintf(chp, "Vertical Axis ");
	}
	else if(*argv[0] == 'L' || *argv[0] == 'l') {
		U8ShellMode = NO_SHELL_MODE;
		latAxisStruct.U8FreezeAxis = DISABLED;
		chprintf(chp, "Lateral Axis ");
	}
	else {
		U8ShellMode = NO_SHELL_MODE;
		latAxisStruct.U8FreezeAxis = DISABLED;
		vertAxisStruct.U8FreezeAxis = DISABLED;
		chprintf(chp, "Both Axes ");
	}
	chprintf(chp, "Unfrozen.\r\n");

	if(ManualStatus.Mode == SIGHTLINE_MODE) {
		chprintf(chp, "Mode : Sightline\r\n");
	}
	else {
		chprintf(chp, "Mode : Manual\r\n");
	}
}

/******************************************************************************
 * Function name:	cmd_mode
 *
 * Description:		Selects/displays mode or reverts to mode switch state
 *
 * Arguments:		NULL: Displays current mode
 * 					MANUAL_MODE: Shell Override Mode: Manual
 * 					SIGHTLINE_MODE: Shell Override Mode: Sightline
 * 					NO_SHELL_MODE: Manual Control Box Mode Command
 *
 *****************************************************************************/
void cmd_mode(BaseSequentialStream *chp, int argc, char *argv[]) {

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
		if(ManualStatus.Mode == SIGHTLINE_MODE) {
			chprintf(chp, "Mode : Sightline\r\n");
		}
		else {
			chprintf(chp, "Mode : Manual\r\n");
		}
	}
}

/******************************************************************************
 * Function name:	cmd_gain
 *
 * Description:		Displays/modifies PID feedback coefficients
 *
 * Arguments:		l p: Displays lateral proportional coefficient
 * 					l i: Displays lateral integral coefficient
 * 					l d: Displays lateral derivative coefficient
 *
 * 					v p: Displays vertical proportional coefficient
 * 					v i: Displays vertical integral coefficient
 * 					v d: Displays vertical derivative coefficient
 *
 * 					l p #: Sets lateral proportional coefficient to #
 * 					l i #: Sets lateral integral coefficient to #
 * 					l d #: Sets lateral derivative coefficient to #
 *
 * 					v p #: Sets vertical proportional coefficient to #
 * 					v i #: Sets vertical integral coefficient to #
 * 					v d #: Sets vertical derivative coefficient to #
 *
 *****************************************************************************/
void cmd_gain(BaseSequentialStream *chp, int argc, char *argv[]) {

CONTROL_AXIS_STRUCT *axis_p = NULL;

	(void)argv;
	if (argc > 0) {

		U8ShellEnable = DISABLED;
		chprintf(chp, "System Disabled. Type 'enable' to resume. \r\n");

		if (*argv[0] == 'V' || *argv[0] == 'v') {
			axis_p = &vertAxisStruct;
			chprintf(chp, "Vertical Axis ");
		}
		else if(*argv[0] == 'L' || *argv[0] == 'l') {
			axis_p = &latAxisStruct;
			chprintf(chp, "Lateral Axis ");
		}

		if (*argv[1] == 'P' || *argv[1] == 'p') {
			if(argv[2])
				axis_p->U16PositionPGain = atoi(argv[2]);
			chprintf(chp, "P Gain : %d \r\n", axis_p->U16PositionPGain);
		}
		else if(*argv[1] == 'I' || *argv[1] == 'i') {
			if(argv[2])
				axis_p->U16PositionIGain = atoi(argv[2]);
			chprintf(chp, "I Gain : %d \r\n", axis_p->U16PositionIGain);
		}
		else if(*argv[1] == 'D' || *argv[1] == 'd') {
			if(argv[2])
				axis_p->U16PositionDGain = atoi(argv[2]);
			chprintf(chp, "D Gain : %d \r\n", axis_p->U16PositionDGain);
		}
		else {
			chprintf(chp, "\n\tP Gain : %d \r\n", axis_p->U16PositionPGain);
			chprintf(chp, "\tI Gain : %d \r\n", axis_p->U16PositionIGain);
			chprintf(chp, "\tD Gain : %d \r\n", axis_p->U16PositionDGain);
		}
	}
}

/*
 * Command Line Shell Functions
 */
static const ShellCommand commands[] = {
	{"mem", cmd_mem},
	{"threads", cmd_threads},
	{"data", cmd_data},
	{"stop", cmd_stop},
	{"disable", cmd_stop},
	{"enable", cmd_enable},
	{"mode", cmd_mode},
	{"gain", cmd_gain},
	{"freeze", cmd_freeze},
	{"unfreeze", cmd_unfreeze},
	{NULL, cmd_stop}
};

/*
 * SPI end transfer callback.
 */
static void spicb(SPIDriver *spip) {

	(void)spip;

  /* On transfer end releases the slave select line.*/
  chSysLockFromIsr();
//  spiUnselectI(spip);
  palClearPad(GPIOA, GPIOA_ADC_CNV);
  palTogglePad(GPIOE, GPIOE_PIN6);
  chSysUnlockFromIsr();
  // Weighted Moving Average accumulator of axis feedback samples
  AxisAccumulator[LAT_AXIS] = ((AxisAccumulator[LAT_AXIS] * ADC_ACCUM_WT) + (Samples[LAT_AXIS] * ADC_SAMPLE_WT))/ADC_ACCUM_DIV;
  AxisAccumulator[VERT_AXIS] = ((AxisAccumulator[VERT_AXIS] * ADC_ACCUM_WT) + (Samples[VERT_AXIS] * ADC_SAMPLE_WT))/ADC_ACCUM_DIV;
}

/******************************************************************************
 * Function name:	ReadADCs
 *
 * Description:		Starts a SPI read transaction and reads 4 samples
 *****************************************************************************/
static void extReadADCs(EXTDriver *extp, expchannel_t channel) {

	(void)extp;
	(void)channel;

	chSysLockFromIsr();
	extChannelDisableI(&EXTD1, 6);
	palTogglePad(GPIOE, GPIOE_PIN5);
	spiStartReceiveI(&SPID1, AXIS_ADC_COUNT, (uint16_t *)Samples);
	chSysUnlockFromIsr();
}

/*
 * Starts new conversion of axis input ADCs.
 */
static void convert_start(GPTDriver *gptp) {

	(void) gptp;

	//Read Input ADCs
	chSysLockFromIsr();
	palClearPad(GPIOA, GPIOA_ADC_CNV);
	extChannelEnableI(&EXTD1, 6);
	palSetPad(GPIOA, GPIOA_ADC_CNV);
	watchdog();
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

    SendDiagnostics(&latDiagnostics, &vertDiagnostics);
}

//static void evtReceiveManual(eventid_t id UNUSED){
//
//	ReceiveManual(&ManualStatus);
////	if(ReceiveManual(&ManualStatus) == 1) {
////		ManualWatchdog = 0;
////	}
////	else
////		++ManualWatchdog;
//}

//static void evtReceiveSLA(eventid_t id UNUSED){
//
//    ReceiveSLA(&SLAStatus);
//    Process_SLA(&SLAStatus, &latAxisStruct, &vertAxisStruct);
//}

WORKING_AREA(wa_rx, 512);
msg_t rx_thread(void *p UNUSED) {
    chRegSetThreadName("rx");
    /*
     * This thread creates a UDP socket and then listens for any incoming
     * message, printing it out over serial USB
     */



    //read data from socket
    while(TRUE) {
		if(ReceiveManual(&ManualStatus) == 1) {
			ManualWatchdog = 0;
		}
		else
			++ManualWatchdog;
//		if(ManualStatus.Mode == SIGHTLINE_MODE) {
			ReceiveSLA(&SLAStatus);
//		}
    	chThdSleepMicroseconds(500);
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


	// Read Feedback ADC's
	latAxisStruct.U16FeedbackADC = (axissample_t)AxisAccumulator[LAT_AXIS];
	vertAxisStruct.U16FeedbackADC = (axissample_t)AxisAccumulator[VERT_AXIS];
	AxisAccumulator[VERT_AXIS] = 0;
	AxisAccumulator[LAT_AXIS] = 0;


	if(count >= 75) {
		// Send Neutral Status to Manual Control Box
		SetDiagnostics(&latDiagnostics, &latAxisStruct);
		SetDiagnostics(&vertDiagnostics, &vertAxisStruct);
		chEvtBroadcastI(&ReadyNeutral);
		chEvtBroadcastI(&ReadyDiagnostics);
		NeutralStatus.vertNeutral = ~NeutralStatus.vertNeutral;
		NeutralStatus.latNeutral = ~NeutralStatus.vertNeutral;
		count = 0;
	}
	else
	++count;

//	// Check for SLA message and calculate desired positions
//	if(ManualStatus.Mode == SIGHTLINE_MODE) {
//		chEvtBroadcastI(&ReadySLA);
//	}

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
 * Function name:	ControlAxis
 *
 * Description:		Updates PWM outputs based on system state
 *****************************************************************************/
//void ControlAxis(CONTROL_AXIS_STRUCT *axis_p, adcsample_t sample, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN) {
void ControlAxis(CONTROL_AXIS_STRUCT *axis_p, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN) {

    chSysLockFromIsr();

    // Run control loop
	controlLoop(axis_p);

	// Set axis PWM's
	if(axis_p->S16OutputCommand > 0){ //Forward
		// Compute ON time scaled 0 - 511 counts
		// Can't go to 100% DC, so max is 98%, or 958
		u32Temp = ((uint32_t)(axis_p->S16OutputCommand * (2 * 958))) / 1000;
		pwmEnableChannelI(&PWMD4, PWM_U_CHAN - 1, microsecondsToPWMTicks( u32Temp ));	// U-Pole
		pwmEnableChannelI(&PWMD4, PWM_V_CHAN - 1, microsecondsToPWMTicks( 0 ));			// V-Pole
	}else{ // Reverse
		// Compute ON time scaled 0 - 511 counts
		// Can't go to 100% DC, so max is 98%, or 958
		u32Temp = ((uint32_t)(axis_p->S16OutputCommand * (-2 * 958))) / 1000;
		pwmEnableChannelI(&PWMD4, PWM_U_CHAN - 1, microsecondsToPWMTicks( 0 ));			// U-Pole
		pwmEnableChannelI(&PWMD4, PWM_V_CHAN - 1, microsecondsToPWMTicks( u32Temp ));	// V-Pole
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

    /*
    * Shell manager initialization.
    */
	usbSerialShellStart(commands);

	adcStart(&ADCD1, NULL);

	/*
	* Activates the EXT driver 1.
	* This is for the external interrupt
	*/
	extStart(&EXTD1, &extcfg);

	// Configure pins for PWM output (D12-D15: TIM4, channel 1-4)
	palSetPadMode(GPIOD, GPIOD_VERT_V, PAL_MODE_ALTERNATE(2));	//U-pole, short lead
	palSetPadMode(GPIOD, GPIOD_VERT_U, PAL_MODE_ALTERNATE(2));	//V-pole, short lead
	palSetPadMode(GPIOD, GPIOD_LAT_V, PAL_MODE_ALTERNATE(2));	//U-pole, long lead
	palSetPadMode(GPIOD, GPIOD_LAT_U, PAL_MODE_ALTERNATE(2));	//V-pole, long lead

	pwmStart(&PWMD4, &pwmcfg);

	NeutralStatus.latNeutral = 0;
	NeutralStatus.vertNeutral = 0;



	// Set axis control gain and limit values
	// Set Vertical Axis Gains
	vertAxisStruct.U16PositionPGain = 60;
	vertAxisStruct.U16PositionIGain = 0;
	vertAxisStruct.U16PositionDGain = 0;
	// Set vertical axis limits
	vertAxisStruct.U16CommandLimit = VERTICAL_COMMAND_LIMIT;
	vertAxisStruct.U16HighPosnLimit = VERT_HIGH_POS_MAX;
	vertAxisStruct.U16LowPosnLimit = VERT_LOW_POS_MAX;
	// Set Lateral Axis Gains
	latAxisStruct.U16PositionPGain = 30;
	latAxisStruct.U16PositionIGain = 0;
	latAxisStruct.U16PositionDGain = 0;
	// Set lateral axis limits
	latAxisStruct.U16CommandLimit = LATERAL_COMMAND_LIMIT;
	latAxisStruct.U16HighPosnLimit = LAT_HIGH_POS_MAX;
	latAxisStruct.U16LowPosnLimit = LAT_LOW_POS_MAX;


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
 * Function name:	microsecondsToPWMTicks
 *
 * Description:		Returns number of PWM ticks in microseconds variable
 *
 *****************************************************************************/
uint32_t microsecondsToPWMTicks(const uint32_t microseconds)
{
	uint32_t ret = (pwmcfg.frequency / 1000000) * microseconds;
	return (ret);
}


/******************************************************************************
 * Function name:	nanosecondsToPWMTicks
 *
 * Description:		Returns number of PWM ticks in nanoseconds variable
 *
 *****************************************************************************/
uint32_t nanosecondsToPWMTicks(const uint32_t nanoseconds) {

	uint32_t ret = (pwmcfg.frequency * nanoseconds) / 1000000000;
	return (ret);
}

/******************************************************************************
 * Function name:	watchdog
 *
 * Description:		Toggles the Watchdog outputs to the GMD's
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
 * Function name:	enable_axes
 *
 * Description:		Asserts or deasserts all axis Enable outputs to the GMD's
 * 					based on the value of enable.
 *
 *****************************************************************************/
static void enable_axes(uint8_t enable) {

	// Enable all axes
	if(enable == ENABLED){
		palSetPad(GPIOD, GPIOD_LAT_EN);
		palSetPad(GPIOD, GPIOD_VERT_EN);
	}
	// Disable all axes
	else
		palClearPad(GPIOD, GPIOD_LAT_EN);
		palClearPad(GPIOD, GPIOD_VERT_EN);
}

/******************************************************************************
 * Function name:	DriveEnable
 *
 * Description:		DANGER - LIFE-SAFETY CRITICAL CODE - THIS CODE WAS
 * 					DEVELOPED BASED ON RESULTS OF SYSTEM FMEA AND SHOULD BE
 * 					MODIFIED ONLY BY QUALIFIED PERSONNEL.
 *
 * 					Performs safety interlock checks before enabling GMDs and
 * 					toggling watchdog as appropriate.
 *
 * 					Returns ENABLED for Drives Enabled
 * 					Returns DISABLED for Drives Disabled
 *****************************************************************************/
static uint8_t DriveEnable() {

uint8_t UserEnable = DISABLED;
uint8_t TestEnable = ENABLED;
uint8_t ApproveEnable = DISABLED;

uint8_t ModeStatus = MANUAL_MODE;

	// Check Mode Status Before Interlocking Drives
	if(U8ShellMode == NO_SHELL_MODE)
		ModeStatus = ManualStatus.Mode;
	else
		ModeStatus = U8ShellMode;

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

//	TestEnable = ENABLED;

	// Check for out-of-range lat axis positions
	if(Samples[LAT_AXIS] >= LAT_HIGH_MAX) {
		++latOutofrangehigh;
		latOutofrangelow = 0;
	}
	else if(Samples[LAT_AXIS] <= LAT_LOW_MAX) {
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
	else if(Samples[VERT_AXIS] <= VERT_LOW_MAX) {
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
	if(ManualWatchdog > MANUAL_WATCHDOG_TIMEOUT) {
		TestEnable = DISABLED;
	}


	// Check all enable flags and approve Enable
	if(UserEnable && TestEnable)
		ApproveEnable = ENABLED;
	else
		ApproveEnable = DISABLED;


	// Handle ENABLE on GMD and Drive Interlocks on Mode Change
	if(ApproveEnable == ENABLED) {

		palClearPad(GPIOE, GPIOE_PIN3);
		// Turn ON enable
		enable_axes(ENABLED);
		// Run watchdog
		watchdog();

		// Interlock drives on mode change
		if(ModeStatus != U8PrevPosnVelModeSwitchState) {
			vertAxisStruct.U8DriveIsInterlocked = ENABLED;
			latAxisStruct.U8DriveIsInterlocked = ENABLED;
		}
		else {
			vertAxisStruct.U8DriveIsInterlocked = DISABLED;
			latAxisStruct.U8DriveIsInterlocked = DISABLED;
		    //Set Desired Positions
		}
		// Set Neutral Flags
//		NeutralStatus.latNeutral = latAxisStruct.U8DriveIsInterlocked;
//		NeutralStatus.vertNeutral = vertAxisStruct.U8DriveIsInterlocked;

		// Turn on Enable LED
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
		// Set Neutral Flags
		NeutralStatus.latNeutral = DISABLED;
		NeutralStatus.vertNeutral = DISABLED;
	}

	// Record previous Mode to support drive interlock on mode change
	U8PrevPosnVelModeSwitchState = ModeStatus;

	// Read Reference Voltage Monitor ADC
	chSysLockFromIsr();
	// Read lat input ADC
	adcStartConversionI(&ADCD1, &adcgrpcfg1, &refMonitor, 1);
	chSysUnlockFromIsr();

	latAxisStruct.U16PositionDesired = ManualStatus.latPosition;
	vertAxisStruct.U16PositionDesired = ManualStatus.vertPosition;

	return ApproveEnable;
}
