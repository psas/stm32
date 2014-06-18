/*
 * rtx_utils.c
 *
 *  Created on: Jun 14, 2014
 *      Author: rgaskell
 */



#include <stdint.h>


/******************************************************************************
 * Function name:	usecondsToPWMTicks
 *
 * Description:		Returns number of PWM ticks in microseconds variable
 *
 *****************************************************************************/
uint32_t usecondsToPWMTicks(const uint32_t microseconds, const int frequency)
{
	uint32_t ret = (frequency / 1000000) * microseconds;
	return (ret);
}


/******************************************************************************
 * Function name:	nanosecondsToPWMTicks
 *
 * Description:		Returns number of PWM ticks in nanoseconds variable
 *
 *****************************************************************************/
uint32_t nanosecondsToPWMTicks(const uint32_t nanoseconds, const int frequency) {

	uint32_t ret = (frequency * nanoseconds) / 1000000000;
	return (ret);
}
