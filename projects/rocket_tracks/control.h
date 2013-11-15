/******************************************************************************
 * File name:		control.h
 *
 * Description:		Header file for control.c
 *
 * Author:		Dan Kirkpatrick
 * Date:		3/10/2012
 *****************************************************************************/
#ifndef _CONTROL_H
#define _CONTROL_H

#include "ch.h"
#include "hal.h"

typedef struct {
	uint8_t U8FreezeAxis;
	uint8_t U8PosnVelMode;
	uint8_t U8DriveIsInterlocked;
	uint8_t U8PositionNeutral;
	uint8_t U8VelocityNeutral;

	adcsample_t U16InputADC;
	adcsample_t U16FeedbackADC;
	adcsample_t U16FeedbackADCPrevious;

	int16_t S16OutputCommand;

	int16_t S16PositionDesired;
	int16_t S16PositionActual;
	int16_t S16PositionError;

	uint16_t S16PositionErrorPrevious;

	int32_t S32PositionPTerm;
	int32_t S32PositionITerm;
	int32_t S32PositionDTerm;
	int32_t S32PositionIAccumulator;

	uint16_t U16PositionPGain;
	uint16_t U16PositionIGain;
	uint16_t U16PositionDGain;
	uint16_t U16LowPosnLimit;
	uint16_t U16HighPosnLimit;
	uint16_t U16CommandLimit;

	int32_t S32PositionDesiredAccumulator;
	int16_t S16VelocityDesired;
	int16_t S16VelocityActual;
	int16_t S16VelocityError;

	int32_t S32VelocityPTerm;
	int32_t S32VelocityITerm;
	int32_t S32VelocityIAccumulator;

	uint16_t U16VelocityPGain;
	uint16_t U16VelocityIGain;
	uint16_t U16VelocityDGain;

}CONTROL_AXIS_STRUCT;

void controlLoop(CONTROL_AXIS_STRUCT * ptr);

#endif
