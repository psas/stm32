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

#define LAT_AXIS_LENGTH						//TODO measure length to payload COM
#define VERT_AXIS_MOTOR_LENGTH				//TODO measure length to lat axis motor from vert axis spindle
#define MOTOR_I								//TODO calculate motor COM moment of inertia
#define MASS								//TODO measure mass of payload
#define MASS_I								//TODO calculate payload COM moment of inertia

typedef uint16_t axissample_t;

typedef struct {

	// Mode and lockout variables
	uint8_t U8FreezeAxis;
	uint8_t U8DriveIsInterlocked;
	uint8_t U8PositionNeutral;
	uint8_t U8VelocityNeutral;

	axissample_t U16FeedbackADC;
	axissample_t U16FeedbackADCPrevious;

	int16_t S16OutputCommand;				//PWM on-time

	int16_t S16PositionDesired;
	int16_t S16PositionActual;
	int16_t S16PositionError;

	uint16_t S16PositionErrorPrevious;

	uint16_t U16MomentofInertia;			//Axis moment of inertia

	int32_t S32PositionPTerm;				//Proportional Feedback
	int32_t S32PositionITerm;				//Integral Feedback
	int32_t S32PositionDTerm;				//Derivative Feedback
	int32_t S32PositionIAccumulator;

	uint16_t U16PositionPGain;				//Proportional Gain
	uint16_t U16PositionIGain;				//Integral Gain
	uint16_t U16PositionDGain;				//Derivative Gain

	// Axis Position Stop Limits
	uint16_t U16LowPosnLimit;
	uint16_t U16HighPosnLimit;

	uint16_t U16CommandLimit;				//PWM on-time high limit

	//
	int32_t S32PositionDesiredAccumulator;	//
	int16_t S16VelocityDesired;
	int16_t S16VelocityActual;
	int16_t S16VelocityError;

} CONTROL_AXIS_STRUCT;

void controlLoop(CONTROL_AXIS_STRUCT * ptr);
void Process_SLA(SLAData * data,
		CONTROL_AXIS_STRUCT * latp, CONTROL_AXIS_STRUCT * vertp);

#endif
