/* \file psas_rtc.h
 *
 */

#ifndef PSAS_RTC_H_
#define PSAS_RTC_H_

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

    extern  psas_rtc_state     psas_rtc_s;
    extern  bool               fs_ready;

    void   psas_rtc_lld_init(void) ;

    void   psas_rtc_to_psas_ts(psas_timespec* ts, RTCTime* rtc) ;
    void   psas_ts_to_psas_rtc(RTCTime* rtc, psas_timespec* ts) ;

    void   psas_stm32_rtc_bcd2tm(struct tm *timp, RTCTime *timespec) ;
    time_t psas_rtc_dr_tr_to_unixtime(RTCTime* timespec) ;
    void   psas_rtc_lld_get_time( RTCDriver *rtcp, RTCTime *timespec) ;
    int    psas_rtc_get_unix_time( RTCDriver *rtcp, RTCTime *timespec) ;
    void   psas_rtc_lld_set_time( RTCDriver *rtcp, RTCTime *timespec) ;


#ifdef __cplusplus
}
#endif

#endif

//! @}

