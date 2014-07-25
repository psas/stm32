/******************************************************************************
 * File name:           control.c
 *
 * Description:         Provides PID position control of an axis in the "Rocket-
 *                      Tracks" antenna pointer system. When in velocity mode,
 *                      this code keeps a desired position variable that is
 *                      increased or decreased based on position of the input
 *                      control lever.
 *
 * Author:              Dan Kirkpatrick
 * Date:                3/10/2012
 *****************************************************************************/

#include <stdint.h>

#include "rocket_tracks.h"
#include "enet_api.h"
#include "control.h"

void controlLoop(CONTROL_AXIS_STRUCT * ptr)
{

	// ************* Position Loop *************

	// Limit Desired position
	if(ptr->U16PositionDesired > ptr->U16HighPosnLimit) {
		ptr->U16PositionDesired = ptr->U16HighPosnLimit;
//		ptr->U16PositionDesired = ptr->U16HighPosnLimit - POSITION_OFFSET;
	}
	else if(ptr->U16PositionDesired < ptr->U16LowPosnLimit) {
		ptr->U16PositionDesired = ptr->U16LowPosnLimit;
//		ptr->U16PositionDesired = ptr->U16LowPosnLimit + POSITION_OFFSET;
	}

	// Compute actual position
	ptr->U16PositionActual = (adcsample_t)ptr->U16FeedbackADC;

	// Compute position error Negative to deal with potentiometer direction
	ptr->S16PositionError = -(ptr->U16PositionDesired - ptr->U16PositionActual);

	// Compute Proportional Term
	ptr->S32PositionPTerm = (ptr->S16PositionError * ptr->U16PositionPGain)/1024;

	// Compute Integral Term and Saturate Result
	ptr->S32PositionIAccumulator = ptr->S32PositionIAccumulator +
			((int32_t)ptr->S16PositionError * (int32_t)ptr->U16PositionIGain);
	if(ptr->S32PositionIAccumulator > (2048*ptr->U16CommandLimit)){
		ptr->S32PositionIAccumulator = (2048*ptr->U16CommandLimit);
	}
	else if(ptr->S32PositionIAccumulator < (-2048*ptr->U16CommandLimit)){
		ptr->S32PositionIAccumulator = (-2048*ptr->U16CommandLimit);
	}
	ptr->S32PositionITerm = ptr->S32PositionIAccumulator / 2048;

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
} // End void controlLoop(CONTROL_AXIS_STRUCT * ptr)

void Process_SLA(SLAData * data, CONTROL_AXIS_STRUCT * latp, CONTROL_AXIS_STRUCT * vertp) {

	int16_t col_coord = 0;
	int16_t row_coord = 0;

	// Calculate Lateral Axis desired position
	col_coord = (data->Column-(COL_PIXELS/2));
	latp->U16PositionDesired = (col_coord * COORD_TO_LAT) + latp->U16PositionActual;

	// Calculate Vertical Axis desired position
	row_coord = (data->Row-(ROW_PIXELS/2));
	vertp->U16PositionDesired = (row_coord * COORD_TO_VERT) + vertp->U16PositionActual;
}

