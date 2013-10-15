/* @file psas_rtc.h
 *
 */

#ifndef _PSAS_RTC_H
#define _PSAS_RTC_H

/*!
 * \addtogroup psas_rtc
 * @{
 */

#include <stdbool.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "rtc.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PSAS_RTC_COE_DEBUG
    #define     PSAS_RTC_COE_DEBUG      0          // Enable the calibration output on PC13
#endif

typedef struct psas_rtc_state {
  bool                  initialized;
} psas_rtc_state;

typedef struct psas_timespec {
    uint8_t PSAS_ns[6];
} psas_timespec;


extern psas_rtc_state     psas_rtc_s;

void psas_rtc_lld_init(void) ;

void psas_stm32_rtc_bcd2tm(struct tm *timp, RTCTime *timespec) ;
void psas_rtc_lld_get_time( RTCDriver *rtcp, RTCTime *timespec) ;


#ifdef __cplusplus
}
#endif

#endif

//! @}

