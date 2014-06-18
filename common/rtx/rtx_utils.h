/*
 * rtx_utils.h
 *
 *  Created on: Jun 14, 2014
 *      Author: rgaskell
 */

#ifndef RTX_UTILS_H_
#define RTX_UTILS_H_

#include <stdint.h>


uint32_t usecondsToPWMTicks(const uint32_t microseconds, const int frequency);
uint32_t nanosecondsToPWMTicks(const uint32_t nanoseconds, const int frequency);

#endif /* RTX_UTILS_H_ */
