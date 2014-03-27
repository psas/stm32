/******************************************************************************
 * File name:		control.c
 *
 * Description:		Provides PID position control of an axis in the "Rocket-
 * 					Tracks" antenna pointer system. When in velocity mode,
 * 					this code keeps a desired position variable that is
 * 					increased or decreased based on position of the input
 * 					control lever.
 *
 * Author:		Dan Kirkpatrick
 * Date:		3/10/2012
 *****************************************************************************/
#include <stdint.h>
#include <math.h>
#include "control.h"


void controlLoop(CONTROL_AXIS_STRUCT * ptr)
{
	// Accumulator scaling multiplier
	const uint16_t U16PosDesAccumMultiplyer = 2048;
	const uint16_t U16PosDesAccumExponentialDivisor = 1024;
	const uint16_t U16VelocityDeadband = 80;

	// ************* Position Loop *************

	// Compute Desired Position
	//ptr->S16PositionDesired

	// Compute actual position
	ptr->S16PositionActual = (adcsample_t)ptr->U16FeedbackADC + 2048;

	//Calculate Gains
	vertAxisStruct.U16MomentofInertia = vertInertia();
	latGains(&latAxisStruct);
	vertGains(&vertAxisStruct);

	// Compute position error Negative to deal with potentiometer direction
	ptr->S16PositionError = -(ptr->S16PositionDesired - ptr->S16PositionActual);

	// Compute Proportional Term
	ptr->S32PositionPTerm = ptr->S16PositionError * ptr->U16PositionPGain;

	// Compute Integral Term and Saturate Result
	ptr->S32PositionIAccumulator = ptr->S32PositionIAccumulator +
			((int32_t)ptr->S16PositionError * (int32_t)ptr->U16PositionIGain);
	if(ptr->S32PositionIAccumulator > (1024*ptr->U16CommandLimit)){
		ptr->S32PositionIAccumulator = (1024*ptr->U16CommandLimit);
	}
	else if(ptr->S32PositionIAccumulator < (-1024*ptr->U16CommandLimit)){
		ptr->S32PositionIAccumulator = (-1024*ptr->U16CommandLimit);
	}
	ptr->S32PositionITerm = ptr->S32PositionIAccumulator / 1024;

	// Compute Derivative Term and Saturate Result
	ptr->S32PositionDTerm = ( (ptr->S16PositionError - ptr->S16PositionErrorPrevious) *
			ptr->U16PositionDGain) / 40;
	if(ptr->S32PositionDTerm > ptr->U16CommandLimit){
		ptr->S32PositionDTerm = ptr->U16CommandLimit;
	}
	else if(ptr->S32PositionDTerm < -ptr->U16CommandLimit){
		ptr->S32PositionDTerm = -ptr->U16CommandLimit;
	}
	ptr->S16PositionErrorPrevious = ptr->S16PositionError;

	// ************* End Position Loop *************

	// Sum Position Terms
	ptr->S16OutputCommand = ptr->S32PositionPTerm;
	ptr->S16OutputCommand += ptr->S32PositionITerm;
	ptr->S16OutputCommand += ptr->S32PositionDTerm;


	// Limit command to allowed range
	if(ptr->S16OutputCommand > ptr->U16CommandLimit){
		ptr->S16OutputCommand = ptr->U16CommandLimit;
	}else if(ptr->S16OutputCommand < -ptr->U16CommandLimit){
		ptr->S16OutputCommand = -ptr->U16CommandLimit;
	}


	// Limit position and check open circuit on sensor
	if(ptr->U16FeedbackADC < 5){
		ptr->S16OutputCommand = 0;
	}
	else if( (ptr->U16FeedbackADC > ptr->U16HighPosnLimit - 2048) &&
			 (ptr->S16OutputCommand < 0) ){
		ptr->S16OutputCommand = 0;
	}
	else if( (ptr->U16FeedbackADC < ptr->U16LowPosnLimit - 2048) &&
			 (ptr->S16OutputCommand > 0) ){
		ptr->S16OutputCommand = 0;
	}

} // End void controlLoop(CONTROL_AXIS_STRUCT * ptr)

float vertInertia() {

	//TODO calculate vertical axis moment of inertia
}

void vertGains() {

	//TODO calculate P gain
	vertAxisStruct.U16PositionPGain;

	//TODO calculate I gain
	vertAxisStruct.U16PositionIGain;

	//TODO calculate D gain
	vertAxisStruct.U16PositionDGain;
}

void latGains() {

	//TODO calculate P gain
	latAxisStruct.U16PositionPGain;

	//TODO calculate I gain
	latAxisStruct.U16PositionIGain;

	//TODO calculate D gain
	latAxisStruct.U16PositionDGain;
}
