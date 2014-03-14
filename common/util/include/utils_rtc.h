/*
 * Convenience functions for the RTC - time format conversions and the like.
 *
 */

#ifndef UTIL_RTC_H_
#define UTIL_RTC_H_

#include "hal.h"

uint64_t rtc_to_ns(RTCTime * timespec);

#endif /* UTIL_RTC_H_ */
