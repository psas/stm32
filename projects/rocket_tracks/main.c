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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

//#include "usb_cdc.h"
#include "chprintf.h"
#include "shell.h"

#include "usbdetail.h"
#include "pwmdetail.h"

#include "main.h"
#include "control.h"

#define VERTICAL_COMMAND_LIMIT		300
#define LATERAL_COMMAND_LIMIT		250

#define LED_OFF						0
#define LED_ON						1
#define LED_BLINK					2

/* Total number of feedback channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   1
/* Depth of the conversion buffer, channels are sampled once each.*/
#define ADC_GRP1_BUF_DEPTH      1
/* Total number of input channels to be sampled by a single ADC operation.*/
#define ADC_GRP2_NUM_CHANNELS   2
/* Depth of the conversion buffer, input channels are sampled once each.*/
#define ADC_GRP2_BUF_DEPTH      1


static void adccb1(ADCDriver *adcp, adcsample_t *buffer, size_t n);
static void adccb2(ADCDriver *adcp, adcsample_t *buffer, size_t n);
static void adccb3(ADCDriver *adcp, adcsample_t *buffer, size_t n);
void DisplayData(BaseSequentialStream *chp, CONTROL_AXIS_STRUCT *axis_p);
static void ControlAxis(CONTROL_AXIS_STRUCT *ptr, adcsample_t sample, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN);
static void motordrive(GPTDriver *gptp);
static void processLeds(void);
uint32_t microsecondsToPWMTicks(const uint32_t microseconds);
//uint32_t nanosecondsToPWMTicks(const uint32_t nanoseconds);
uint32_t pwmGetPclk(void);

static const GPTConfig gpt1cfg = {
	1000000,
//	10000000,
//	motordrive
	NULL
};

static const ADCConversionGroup adcgrp1cfg5 = {
	FALSE,
	ADC_GRP1_NUM_CHANNELS,
	adccb1,
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

static const ADCConversionGroup adcgrp1cfg4 = {
	FALSE,
	ADC_GRP1_NUM_CHANNELS,
	adccb2,
	NULL,
	/* HW dependent part.*/
	0,
	ADC_CR2_SWSTART,
	0,
	ADC_SMPR2_SMP_AN4(ADC_SAMPLE_56),
	ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
	0,
	ADC_SQR3_SQ1_N(ADC_CHANNEL_IN4)
};

static const ADCConversionGroup adcgrp2cfg = {
	FALSE,
	ADC_GRP2_NUM_CHANNELS,
	adccb3,
	NULL,
	/* HW dependent part.*/
	0,
	ADC_CR2_SWSTART,
	0,
	ADC_SMPR2_SMP_AN4(ADC_SAMPLE_112) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_112),
	ADC_SQR1_NUM_CH(ADC_GRP2_NUM_CHANNELS),
	0,
	ADC_SQR3_SQ2_N(ADC_CHANNEL_IN4) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN5)
};

// Global variables

static adcsample_t InputSamples[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
static adcsample_t vertFeedbackSample[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static adcsample_t latFeedbackSample[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

uint8_t U8EnableDriveSwitch = 0;
uint8_t U8PrevPosnVelModeSwitchState = 0;

uint8_t U8ShellEnable = 0;
uint8_t U8ShellMode = 2;
//uint8_t U8trackupflag = 0;

uint32_t u32Temp = 0;
static uint8_t u8WatchDogCycleCount = 0;

static uint32_t U32DelayCount = 0;
static uint8_t U8PosnVelModeSwitch = 0;
static uint32_t u32LedCount = 0;
static uint8_t U8LedState = 0;
static uint8_t U8VertLedState = LED_BLINK;
static uint8_t U8LatLedState = LED_BLINK;
CONTROL_AXIS_STRUCT vertAxisStruct, latAxisStruct;


/* Olimex stm32-e407 board */

static EventSource wkup_event;

/*! \sa HAL_USE_EXT in hal_conf.h
 */
static void green_led_off(void *arg) {

  (void)arg;
  palSetPad(GPIOC, GPIOC_LED);
}

/* Triggered when the WKUP button is pressed or released. The LED is set to ON.*/

/* Challenge: Add de-bouncing */
static void extcb1(EXTDriver *extp, expchannel_t channel) {
  static VirtualTimer vt4;

  (void)extp;
  (void)channel;

  palClearPad(GPIOC, GPIOC_LED);
  chSysLockFromIsr();
  chEvtBroadcastI(&wkup_event);

  if (chVTIsArmedI(&vt4))
    chVTResetI(&vt4);

  /* LED4 set to OFF after 500mS.*/
  chVTSetI(&vt4, MS2ST(500), green_led_off, NULL);
  chSysUnlockFromIsr();
}

static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1},   // WKUP Button PA0
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
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL}
  }
};


/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

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
static void cmd_data(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv; (void)argc;

	if(U8ShellMode != 2) {
		chprintf(chp, "\tShell Override ");
		if(U8ShellMode == 1) {
			chprintf(chp, "Mode : Position\r\n");
		}
		else {
			chprintf(chp, "Mode : Velocity\r\n");
		}
	}
	else {
		if(U8PosnVelModeSwitch == 1) {
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

	chprintf(chp, " PositionActual : %d counts ", axis_p->S16PositionActual);
	chprintf(chp, "Desired : %d counts\r\n", axis_p->S16PositionDesired);

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
static void cmd_stop(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: stop\r\n");
	}
	U8ShellEnable = 0;
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
static void cmd_enable(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: Enable\r\n");
		return;
	}
	U8ShellEnable = 1;
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
static void cmd_freeze(BaseSequentialStream *chp, int argc, char *argv[]) {

//CONTROL_AXIS_STRUCT *axis_p = NULL;

	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: Freeze\r\n");
		return;
	}
	if (*argv[0] == 'V' || *argv[0] == 'v') {
		U8ShellMode = 3;
		vertAxisStruct.U8FreezeAxis = 1;
		chprintf(chp, "Vertical Axis ");
	}
	else if(*argv[0] == 'L' || *argv[0] == 'l') {
		U8ShellMode = 3;
		latAxisStruct.U8FreezeAxis = 1;
		chprintf(chp, "Lateral Axis ");
	}
	else {
		U8ShellMode = 3;
		latAxisStruct.U8FreezeAxis = 1;
		vertAxisStruct.U8FreezeAxis = 1;
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
static void cmd_unfreeze(BaseSequentialStream *chp, int argc, char *argv[]) {

//CONTROL_AXIS_STRUCT *axis_p = NULL;

	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: Unfreeze\r\n");
		return;
	}
	if (*argv[0] == 'V' || *argv[0] == 'v') {
		U8ShellMode = 1;
		vertAxisStruct.U8FreezeAxis = 0;
		chprintf(chp, "Vertical Axis ");
	}
	else if(*argv[0] == 'L' || *argv[0] == 'l') {
		U8ShellMode = 2;
		latAxisStruct.U8FreezeAxis = 0;
		chprintf(chp, "Lateral Axis ");
	}
	else {
		U8ShellMode = 2;
		latAxisStruct.U8FreezeAxis = 0;
		vertAxisStruct.U8FreezeAxis = 0;
		chprintf(chp, "Both Axes ");
	}
	chprintf(chp, "Unfrozen.\r\n");

	if(U8PosnVelModeSwitch > 0) {
		chprintf(chp, "Mode : Position\r\n");
	}
	else {
		chprintf(chp, "Mode : Velocity\r\n");
	}
}

/******************************************************************************
 * Function name:	cmd_mode
 *
 * Description:		Selects/displays mode or reverts to mode switch state
 *
 * Arguments:		NULL: Displays current mode
 * 					0: Shell Override Mode: Velocity
 * 					1: Shell Override Mode: Position
 * 					2: Mode switch
 *
 *****************************************************************************/
static void cmd_mode(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void)argv;
	if (argc > 0) {
		if (*argv[0] == '0')
			U8ShellMode = 0;
		else if(*argv[0] == '1')
			U8ShellMode = 1;
		else if(*argv[0] == '2')
			U8ShellMode = 2;
	}
	if(U8ShellMode != 2) {
		chprintf(chp, "\tShell Override ");
		if(U8ShellMode > 0) {
			chprintf(chp, "Mode : Position\r\n");
		}
		else {
			chprintf(chp, "Mode : Velocity\r\n");
		}
	}
	else {
		if(U8PosnVelModeSwitch > 0) {
			chprintf(chp, "Mode : Position\r\n");
		}
		else {
			chprintf(chp, "Mode : Velocity\r\n");
		}
	}
}

/******************************************************************************
 * Function name:	cmd_stop
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
static void cmd_gain(BaseSequentialStream *chp, int argc, char *argv[]) {

CONTROL_AXIS_STRUCT *axis_p = NULL;

	(void)argv;
	if (argc > 0) {

		U8ShellEnable = 0;
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


//static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
//	static const char *states[] = {THD_STATE_NAMES};
//	Thread *tp;
//
//	(void)argv;
//	if (argc > 0) {
//		chprintf(chp, "Usage: threads\r\n");
//		return;
//	}
//	chprintf(chp, "    addr    stack prio refs     state time\r\n");
//	tp = chRegFirstThread();
//	do {
//		chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
//			(uint32_t)tp, (uint32_t)tp->p_ctx.r13,
//			(uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
//			states[tp->p_state], (uint32_t)tp->p_time);
//		tp = chRegNextThread(tp);
//	} while (tp != NULL);
//}

/*
 * Challenge: add additional command line functions
 */
static const ShellCommand commands[] = {
	{"mem", cmd_mem},
//	{"threads", cmd_threads},
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
 * ADC1 end conversion callback.
 * The vertical axis control loop is run and PWM updated.
 */
static void adccb1(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

	(void) buffer; (void) n;
	/* Note, only in the ADC_COMPLETE state because the ADC driver fires an
	 intermediate callback when the buffer is half full.*/
	if (adcp->state == ADC_COMPLETE) {
		ControlAxis(&vertAxisStruct, vertFeedbackSample[0], 1, 2);
	}
}

/*
 * ADC2 end conversion callback.
 * The lateral axis control loop is run and PWM updated.
 */
static void adccb2(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

	(void) buffer; (void) n;
	/* Note, only in the ADC_COMPLETE state because the ADC driver fires an
	 intermediate callback when the buffer is half full.*/
	if (adcp->state == ADC_COMPLETE) {
		ControlAxis(&latAxisStruct, latFeedbackSample[0], 3, 4);
	}
}

/*
 * ADC3 end conversion callback.
 * The input values are stored and feedback ADC's read.
 */
static void adccb3(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

	(void) buffer; (void) n;
	/* Note, only in the ADC_COMPLETE state because the ADC driver fires an
	 intermediate callback when the buffer is half full.*/
	if (adcp->state == ADC_COMPLETE) {
		latAxisStruct.U16InputADC = InputSamples[0];
		vertAxisStruct.U16InputADC = InputSamples[1];
	}
}

/*
 * GMD control thread, times are in microseconds.
 */
static void motordrive(GPTDriver *gptp) {

	(void) gptp;

	U32DelayCount = 0;

	chSysLockFromIsr();

	// Read Input ADC's
	adcStartConversionI(&ADCD3, &adcgrp2cfg, InputSamples, ADC_GRP2_BUF_DEPTH);
	// Read Feedback ADC's
	adcStartConversionI(&ADCD1, &adcgrp1cfg5, vertFeedbackSample, ADC_GRP1_BUF_DEPTH);
	adcStartConversionI(&ADCD2, &adcgrp1cfg4, latFeedbackSample, ADC_GRP1_BUF_DEPTH);

	chSysUnlockFromIsr();

//TODO undo the constant enable
	// Read input switch positions
//	U8EnableDriveSwitch = 1;
	U8EnableDriveSwitch = palReadPad(GPIOD, GPIOD_PIN8); // PD8
//	U8PosnVelModeSwitch = 1; // PD9
	U8PosnVelModeSwitch = palReadPad(GPIOD, GPIOD_PIN9); // PD9

	// Handle ENABLE on GMD and LEDs on GFE
	if(U8EnableDriveSwitch > 0 && U8ShellEnable > 0) {
		// Turn ON enable
		palSetPad(GPIOD, GPIOD_PIN10);
		palSetPad(GPIOD, GPIOD_PIN6);
//		// Turn Green LED on, Red LED off
//		palSetPad(GPIOD, GPIOD_PIN0);
//		palClearPad(GPIOD, GPIOD_PIN1);

		// Interlock drives on mode change
		if(U8ShellMode == 2) {
			if(U8PosnVelModeSwitch != U8PrevPosnVelModeSwitchState) {
				vertAxisStruct.U8DriveIsInterlocked = 1;
				latAxisStruct.U8DriveIsInterlocked = 1;
			}
		}
		else {
			if(U8ShellMode != U8PrevPosnVelModeSwitchState) {
				vertAxisStruct.U8DriveIsInterlocked = 1;
				latAxisStruct.U8DriveIsInterlocked = 1;
			}
		}
	}
	else {
		// Otherwise enable is OFF
		palClearPad(GPIOD, GPIOD_PIN10);
		palClearPad(GPIOD, GPIOD_PIN6);
//		// Turn Green LED off, Red LED on
//		palClearPad(GPIOD, GPIOG_PIN0);
//		palSetPad(GPIOD, GPIOD_PIN1);

		// Lock out drives
		vertAxisStruct.U8DriveIsInterlocked = 1;
		latAxisStruct.U8DriveIsInterlocked = 1;
	}

	// Interlock drives on mode change
	if(U8ShellMode == 2) {
		U8PrevPosnVelModeSwitchState = U8PosnVelModeSwitch;
	}
	else {
		U8PrevPosnVelModeSwitchState = U8ShellMode;
	}

//	U8PrevPosnVelModeSwitchState = U8PosnVelModeSwitch;

	processLeds();

	// Handle HW Watchdog on GMD
	if(u8WatchDogCycleCount > 7){
		u8WatchDogCycleCount = 0;
		palTogglePad(GPIOD, GPIOD_PIN11);
		palTogglePad(GPIOD, GPIOD_PIN5);
	}
	else
		u8WatchDogCycleCount++;

	return;
}

/*
 * WKUP button handler
 *
 * Challenge: Do something more interesting here.
 */
static void WKUP_button_handler(eventid_t id) {

	BaseSequentialStream *chp = getActiveUsbSerialStream();
	chprintf(chp, "WKUP btn. eventid: %d\r\n", id);
	chprintf(chp, "STM32_TIMCLK1 is: %d\r\n", STM32_TIMCLK1);
}

/*
 * Application entry point.
 */
int main(void) {
	static const evhandler_t evhndl[] = {
	WKUP_button_handler
	};
	struct EventListener el0;
	/*
	* System initializations.
	* - HAL initialization, this also initializes the configured device drivers
	*   and performs the board-specific initializations.
	* - Kernel initialization, the main() function becomes a thread and the
	*   RTOS is active.
	*/

	halInit();
	chSysInit();

	/*
	* Initialize event structures BEFORE using them
	*/
	chEvtInit(&wkup_event);

    /*
    * Shell manager initialization.
    */
	usbSerialShellStart(commands);

	// Enable Continuous GPT for 1ms Interval
	gptStart(&GPTD1, &gpt1cfg);
	gptStartContinuous(&GPTD1,10000);

	// Configure pins for Feedback ADC's
	palSetPadMode(GPIOA, GPIOA_PIN4, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, GPIOA_PIN5, PAL_MODE_INPUT_ANALOG);
	// Configure pins for Input ADC's
	palSetPadMode(GPIOF, GPIOF_PIN6, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOF, GPIOF_PIN7, PAL_MODE_INPUT_ANALOG);

	// Configure pins for LED's
	// PD3: Vertical Axis LED
	palSetPadMode(GPIOD, GPIOD_PIN3, PAL_MODE_OUTPUT_PUSHPULL);
	// PD4: Lateral Axis LED
	palSetPadMode(GPIOD, GPIOD_PIN4, PAL_MODE_OUTPUT_PUSHPULL);

	// Configure pins for switches
	// PD8: Drive Enable Switch
	palSetPadMode(GPIOD, GPIOD_PIN8, PAL_MODE_INPUT);
	// PD9: Mode Select Switch
	palSetPadMode(GPIOD, GPIOD_PIN9, PAL_MODE_INPUT);

	// Configure pins for long lead GMD Enable/Watchdog
	// PD5: Enable out to GMD
	palSetPadMode(GPIOD, GPIOD_PIN5, PAL_MODE_OUTPUT_PUSHPULL);
	// PD6: Watchdog out to GMD
	palSetPadMode(GPIOD, GPIOD_PIN6, PAL_MODE_OUTPUT_PUSHPULL);

	// Configure pins for short lead GMD Enable/Watchdog
	// PD10: Enable out to GMD
	palSetPadMode(GPIOD, GPIOD_PIN10, PAL_MODE_OUTPUT_PUSHPULL);
	// PD11: Watchdog out to GMD
	palSetPadMode(GPIOD, GPIOD_PIN11, PAL_MODE_OUTPUT_PUSHPULL);

	// Configure pins for PWM output (D12-D15: TIM4, channel 1-4)
	palSetPadMode(GPIOD, GPIOD_PIN12, PAL_MODE_ALTERNATE(2));	//U-pole, short lead
	palSetPadMode(GPIOD, GPIOD_PIN13, PAL_MODE_ALTERNATE(2));	//V-pole, short lead
	palSetPadMode(GPIOD, GPIOD_PIN14, PAL_MODE_ALTERNATE(2));	//U-pole, long lead
	palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));	//V-pole, long lead

	adcStart(&ADCD1, NULL);
	adcStart(&ADCD2, NULL);
	adcStart(&ADCD3, NULL);

	pwmStart(&PWMD4, &pwmcfg);

	// Enable TIM4 PWM channel 1-4 with initial DC=0%
	/* @param[in] pwmp      pointer to a @p PWMDriver object
	*  @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
	*  @param[in] width     PWM pulse width as clock pulses number
	*/
	pwmEnableChannel(&PWMD4, 0, 0);
	pwmEnableChannel(&PWMD4, 1, 0);
	pwmEnableChannel(&PWMD4, 2, 0);
	pwmEnableChannel(&PWMD4, 3, 0);

	// Set axis control gain and limit values
	// Set Vertical Axis Gains
	vertAxisStruct.U16PositionPGain = 4;
	vertAxisStruct.U16PositionIGain = 1;
	vertAxisStruct.U16PositionDGain = 0;
	// Set vertical axis limits
	vertAxisStruct.U16CommandLimit = VERTICAL_COMMAND_LIMIT;
	vertAxisStruct.U16HighPosnLimit = 5100;
	vertAxisStruct.U16LowPosnLimit = 2480;
	// Set Lateral Axis Gains
	latAxisStruct.U16PositionPGain = 2;
	latAxisStruct.U16PositionIGain = 0;
	latAxisStruct.U16PositionDGain = 0;
	// Set lateral axis limits
	latAxisStruct.U16CommandLimit = LATERAL_COMMAND_LIMIT;
	latAxisStruct.U16HighPosnLimit = 5300;
	latAxisStruct.U16LowPosnLimit = 3100;

	/*
	* Activates the serial driver 6 and SDC driver 1 using default
	* configuration.
	*/
	sdStart(&SD6, NULL);

	/*
	* Activates the EXT driver 1.
	* This is for the external interrupt
	*/
	extStart(&EXTD1, &extcfg);

	/*
	* Normal main() thread activity, in this demo it does nothing except
	* sleeping in a loop and listen for events.
	*/
	chEvtRegister(&wkup_event, &el0, 0);
	while (TRUE) {
		//Cycle motordrive if timer fails
		if(U32DelayCount++ > 2500){
			motordrive(&GPTD1);
		}
	chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
	}
}

//void TrackUp() {
//
//	if(vertAxisStruct.S16PositionActual < 3800) {
//		vertAxisStruct.S16PositionDesired = vertAxisStruct.S16PositionDesired + 10;
//	}
//	else {
//
//	}
//}

/******************************************************************************
 * Function name:	ControlAxis
 *
 * Description:		Updates PWM outputs based on system state
 *****************************************************************************/
void ControlAxis(CONTROL_AXIS_STRUCT *axis_p, adcsample_t sample, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN) {
//void ControlAxis(CONTROL_AXIS_STRUCT *axis_p, uint8_t PWM_U_CHAN, uint8_t PWM_V_CHAN) {

	// Get feedback ADC value
	axis_p->U16FeedbackADC = sample;

//	// Get commanded position
//	if(U8trackupflag) {
//		TrackUp();
//	}
	// Set axis mode
	if(U8ShellMode == 2)
		axis_p->U8PosnVelMode = U8PosnVelModeSwitch;
	else
		axis_p->U8PosnVelMode = U8ShellMode;

	// Run control loop
	controlLoop(axis_p);


    chSysLockFromIsr();
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

/******************************************************************************
 * Function name:	processLeds
 *
 * Description:		Manages LEDs based on system state
 *
 *****************************************************************************/
static void processLeds(void){

	// Set LED's to blink if drive is interlocked
	if( (vertAxisStruct.U8VelocityNeutral > 0) | (vertAxisStruct.U8PositionNeutral > 0) ) {
		U8VertLedState = LED_ON;
	}else{
		U8VertLedState = LED_BLINK;
	}
	if( (latAxisStruct.U8VelocityNeutral > 0) | (latAxisStruct.U8PositionNeutral > 0) ) {
		U8LatLedState = LED_ON;
	}else{
		U8LatLedState = LED_BLINK;
	}

	// Handle LED timer operation
	if(u32LedCount > 50){
		u32LedCount = 0;
		U8LedState = ~U8LedState;
	}else{
		u32LedCount++;
	}

	// Process Vertical LED on PD3
	switch(U8VertLedState){
	case LED_OFF:
		palClearPad(GPIOD, GPIOD_PIN3);
		break;
	case LED_ON:
		palSetPad(GPIOD, GPIOD_PIN3);
		break;
	case LED_BLINK:
		if(U8LedState){palSetPad(GPIOD, GPIOD_PIN3);}
		else{palClearPad(GPIOD, GPIOD_PIN3);}
		break;
	}

	// Process Lateral LED on PD4
	switch(U8LatLedState){
	case LED_OFF:
		palClearPad(GPIOD, GPIOD_PIN4);
		break;
	case LED_ON:
		palSetPad(GPIOD, GPIOD_PIN4);
		break;
	case LED_BLINK:
		if(U8LedState){palSetPad(GPIOD, GPIOD_PIN4);}
		else{palClearPad(GPIOD, GPIOD_PIN4);}
		break;
	}

//	// Process Posn / Vel LEDs on PD5 and PD6
//	if(U8PosnVelModeSwitch > 0){
//		palSetPad(GPIOD, GPIOD_PIN5);
//		palClearPad(GPIOD, GPIOD_PIN6);
//	}else{
//		palClearPad(GPIOD, GPIOD_PIN5);
//		palSetPad(GPIOD, GPIOD_PIN6);
//	}
	// Process flashing Green Wake-up LED
	if(U8LedState){palSetPad(GPIOC, GPIOC_LED);}
	else{palClearPad(GPIOC, GPIOC_LED);}

}

/******************************************************************************
 * Function name:	microsecondsToPWMTicks
 *
 * Description:		Returns number of PWM ticks in microseconds variable
 *
 *****************************************************************************/
uint32_t microsecondsToPWMTicks(const uint32_t microseconds)
{
	uint32_t ret = (pwmGetPclk() / 1000000) * microseconds;
	return (ret);
}

/******************************************************************************
 * Function name:	nanosecondsToPWMTicks
 *
 * Description:		Returns number of PWM ticks in nanoseconds variable
 *
 *****************************************************************************/
uint32_t nanosecondsToPWMTicks(const uint32_t nanoseconds) {

	uint32_t ret = (pwmGetPclk() * nanoseconds) / 1000000000;
	return (ret);
}

/******************************************************************************
 * Returns PWM frequency rate so that frequency and duty cycle can be computed.
 *
 *
 *****************************************************************************/
uint32_t pwmGetPclk(void) {

	return pwmcfg.frequency;
}

