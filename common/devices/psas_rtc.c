/*! \file psas_rtc.c
 *
 Some parts of this file under this license:

 ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*!
 * \defgroup psas_rtc PSAS Non-standard RTC
 * @{
 */

/*!
 * These functions are for use when the RTC_SSR register is needed
 * and the prediv_a and prediv_s values are non-standard.
 *
 * RTCCLK Note: LSE clock must be chosen for the RTCCLK if the RTC module is to
 * run from Vbat only. (e.g. VDD is disconnected)
 *  In the ChibiOS file mcuconf.h change this line as shown:
 * // #define STM32_RTCSEL                        STM32_RTCSEL_LSI
 * #define STM32_RTCSEL                        STM32_RTCSEL_LSE
 *
 *
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "ch.h"
#include "hal.h"
#include "rtc.h"

#include "stm32f4xx.h"

#include "chprintf.h"
#include "chrtclib.h"

#include "psas_rtc.h"


#ifdef DEBUG_RTCEXP
#include "usbdetail.h"
BaseSequentialStream    *rtcexp   =  (BaseSequentialStream *)&SDU_PSAS;
#define RTCDBG(format, ...) chprintf( rtcexp, format, ##__VA_ARGS__ )
#else
#define RTCDBG(...)
#endif


/*!
 * \brief   Wait for synchronization of RTC registers with APB1 bus.
 * \details This function must be invoked before trying to read RTC registers.
 */
#define psas_rtc_lld_apb1_sync() {while ((RTCD1.id_rtc->ISR & RTC_ISR_RSF) == 0);}

#define psas_rtc_lld_enter_init() {                                           \
    RTCD1.id_rtc->ISR |= RTC_ISR_INIT;                                          \
    while ((RTCD1.id_rtc->ISR & RTC_ISR_INITF) == 0)                            \
    ;                                                                         \
}


#define PSAS_RTC_INCORRECT_TIME_ERROR            -1
#define PSAS_RTC_ERROR                           -2

#define ONE_BILLION 1000000000


/*!
 * \brief   Finalizing of configuration procedure.
 */
#define psas_rtc_lld_exit_init() {RTCD1.id_rtc->ISR &= ~RTC_ISR_INIT;}

psas_rtc_state     psas_rtc_s = {false, 0};

/*!
 * \brief   Enable access to registers.
 *
 * This API will use the subsecond register
 * at full speed for sub second timestamping.
 *
 * This requires the prescaler registers to be set
 * differently from the ChibiOS default values.
 *
 * For the LSE 32768 clock, this function will result in a fractional
 * read of 61uS per tick. The RTC_SSR register is the value of the
 * 15 bit prescaler. See the ST Reference manual for details.
 *
 * Call this routine early in main to set these values.
 *
 */
void psas_rtc_lld_init(void) {
    RTCD1.id_rtc = RTC;

    /*
     * This prescaler is run from the LSIRC at 32khz
     */
    uint32_t prediv_a = 0x1;  // this is the asynch 7 bit register

    /* Disable write protection. */
    RTCD1.id_rtc->WPR = 0xCA;
    RTCD1.id_rtc->WPR = 0x53;

    /*
       From Ref manual p 634
     * If COSEL is set and “PREDIV_S+1” is a non-zero multiple of 256 (i.e:
     *        PREDIV_S[7:0] = 0xFF), the RTC_CALIB frequency is fRTCCLK/(256 *
     *            (PREDIV_A+1)). This corresponds to a calibration output at 1 Hz for
     *        prescaler default values (PREDIV_A = Ox7F, PREDIV_S = 0xFF), with an
     *        RTCCLK frequency at 32.768 kHz.prediv_a
     *
     * Since prediv_a = 1, The output on pc13 RTC_AF1 is 64hz.
     */

#if PSAS_RTC_COE_DEBUG
    #warning "COE DEBUG Output enabled on RTC_AF1 pin"
    RTCD1.id_rtc->CR   |= RTC_CR_COE;
    RTCD1.id_rtc->CR   |= RTC_CR_COSEL;
#else
    RTCD1.id_rtc->CR   = 0;
#endif

    psas_rtc_lld_enter_init();
    /*
     * Prescaler register must be written in two SEPARATE writes.
     * See page 630 of reference manual
     *
     * To have 1Hz output for ck_spre set
     * preset_s = (STM32_RTCCLK / (prediv_a + 1)) - 1)
     */
    prediv_a = (prediv_a << 16) |
        (((STM32_RTCCLK / (prediv_a + 1)) - 1) & 0x7FFF);
    RTCD1.id_rtc->PRER = prediv_a;
    RTCD1.id_rtc->PRER = prediv_a;

    psas_rtc_lld_exit_init();

    psas_rtc_s.initialized = true;

}

void psas_rtc_set_fc_boot_mark(RTCDriver* rtcp) {
    RTCTime mark_time;

    psas_rtc_get_unix_time(rtcp, &mark_time);
    psas_rtc_s.fc_boot_time_mark = mark_time.tv_time;
}


/*! \brief   Converts from STM32 BCD to canonicalized time format.
 *
 * \param[out] timp     pointer to a @p tm structure as defined in time.h
 * \param[in] timespec  pointer to a @p RTCTime structure
 *
 */
void psas_stm32_rtc_bcd2tm(struct tm *timp, RTCTime *timespec) {
    uint32_t tv_time = timespec->tv_time;
    uint32_t tv_date = timespec->tv_date;

    timp->tm_isdst = -1;

    timp->tm_wday = (tv_date & RTC_DR_WDU) >> RTC_DR_WDU_OFFSET;
    if (timp->tm_wday == 7)
        timp->tm_wday = 0;

    timp->tm_mday =  (tv_date & RTC_DR_DU) >> RTC_DR_DU_OFFSET;
    timp->tm_mday += ((tv_date & RTC_DR_DT) >> RTC_DR_DT_OFFSET) * 10;

    timp->tm_mon  =  (tv_date & RTC_DR_MU) >> RTC_DR_MU_OFFSET;
    timp->tm_mon  += ((tv_date & RTC_DR_MT) >> RTC_DR_MT_OFFSET) * 10;
    timp->tm_mon  -= 1;

    timp->tm_year =  (tv_date & RTC_DR_YU) >> RTC_DR_YU_OFFSET;
    timp->tm_year += ((tv_date & RTC_DR_YT) >> RTC_DR_YT_OFFSET) * 10;
    timp->tm_year += 2000 - 1900;

    timp->tm_sec  =  (tv_time & RTC_TR_SU) >> RTC_TR_SU_OFFSET;
    timp->tm_sec  += ((tv_time & RTC_TR_ST) >> RTC_TR_ST_OFFSET) * 10;

    timp->tm_min  =  (tv_time & RTC_TR_MNU) >> RTC_TR_MNU_OFFSET;
    timp->tm_min  += ((tv_time & RTC_TR_MNT) >> RTC_TR_MNT_OFFSET) * 10;

    timp->tm_hour =  (tv_time & RTC_TR_HU) >> RTC_TR_HU_OFFSET;
    timp->tm_hour += ((tv_time & RTC_TR_HT) >> RTC_TR_HT_OFFSET) * 10;
    timp->tm_hour += 12 * ((tv_time & RTC_TR_PM) >> RTC_TR_PM_OFFSET);
}

/*! \brief Convert from an RTCTime struct to a psas_timespec
 *
 *  \warning rtc is assumed to be in seconds, not the raw register
 *   (BCD-ish) form.
 *
 *  \sa psas_rtc_lld_get_time()
 */
void psas_rtc_to_psas_ts(psas_timespec* ts, RTCTime* rtc) {
    uint64_t total_ns = 0;

    total_ns = rtc->tv_time - psas_rtc_s.fc_boot_time_mark;
    total_ns = total_ns * ONE_BILLION;
    total_ns += (rtc->tv_msec) * 1000;

    int i;
    for (i = 0; i < PSAS_RTC_NS_BYTES; i++) {
        ts->ns[i] = (uint8_t) (total_ns >> (i * 8)) & 0xff;
    }
}


/*! \brief Convert from a psas_timespec to an RTCTime struct
 *
 *  \warning rtc is returned in seconds, not the raw register
 *   (BCD-ish) form.
 *
 *  \sa psas_rtc_lld_get_time()
 */
void psas_ts_to_psas_rtc(RTCTime* rtc, psas_timespec* ts) {
    uint64_t    total_ns_in = 0;
    int         i           = 0;

    for (i = 0; i < PSAS_RTC_NS_BYTES; i++) {
        total_ns_in |= ts->ns[i] << (i * 8);
    }

    rtc->tv_time = (uint32_t) (total_ns_in / ONE_BILLION);
    rtc->tv_msec = (uint32_t) (total_ns_in % ONE_BILLION) / 1000;
}

/*! \brief Utility function to convert from RTC TR register
 *
 * Return the simple calendar time representation from a
 * RTC DR and TR register read.
 */
time_t psas_rtc_dr_tr_to_unixtime(RTCTime* timespec) {
    time_t        unix_time;
    struct   tm   timp;

    psas_stm32_rtc_bcd2tm(&timp, timespec);
    unix_time = mktime(&timp);

    return unix_time;
}

/*!
 * \brief   Get current time.
 * Returns time in format from RTC{TR,DR} registers
 * timespec->tv_msec is in milliseconds.
 *
 * \param[in]  rtcp      pointer to RTC driver structure
 * \param[out] timespec  pointer to a @p RTCTime structure
 *
 * Returns raw data from TR and DR. Consider using
 *
 *       time_t        unix_time;
 *       struct   tm   timp;
 *       RTCTime       psas_time;
 *      (...)
 *        psas_rtc_lld_get_time(&RTCD1, &psas_time);
 *        psas_stm32_rtc_bcd2tm(&timp, &psas_time);
 *        unix_time = mktime(&timp);
 *      (...)
 *
 * This is how the ChibiOS function rtc_lld_get_time() works.
 * This function attempts to parallel that behavior.
 *
 * \sa psas_tr_to_unixtime
 *
 */
void psas_rtc_lld_get_time( RTCDriver *rtcp, RTCTime *timespec) {
    (void)rtcp;

    if(!psas_rtc_s.initialized) {
        timespec->tv_time = 0;
        timespec->tv_date = 0;
        return;
    }

    psas_rtc_lld_apb1_sync();

#if STM32_RTC_HAS_SUBSECONDS
    timespec->tv_msec =1000000 * ((1.0 * (RTCD1.id_rtc->PRER & 0x7FFF) - RTCD1.id_rtc->SSR) /
            ((RTCD1.id_rtc->PRER & 0x7FFF) + 1));
#endif /* STM32_RTC_HAS_SUBSECONDS */
    timespec->tv_time = RTCD1.id_rtc->TR;   // \warning RAW register format
    timespec->tv_date = RTCD1.id_rtc->DR;   // See Ref Manual: p637
}


/*! \brief utility function to convert the TR and DR register
 *   data to simple time
 *
 * time returned to the RTCTime tv_time member.
 * function returns 0 on success
 */
int psas_rtc_get_unix_time( RTCDriver *rtcp, RTCTime *timespec) {
    time_t    unix_time;
    RTCTime   psas_time;

    psas_rtc_lld_get_time(rtcp, &psas_time);

    unix_time = psas_rtc_dr_tr_to_unixtime(&psas_time);

    if(unix_time  == -1) {
        return PSAS_RTC_INCORRECT_TIME_ERROR;
    }

    timespec->tv_time = unix_time;

    return 0;
}

/*!
 * \brief set current time
 *
 * \param[in]  rtcp      pointer to RTC driver structure
 * \param[in]  timespec pointer to a \p RTCTime structure
 */
void psas_rtc_lld_set_time( RTCDriver *rtcp, RTCTime *timespec) {

    if(!psas_rtc_s.initialized) {
        return;
    }

    // will convert to proper BCD register format for STM32
    rtcSetTimeUnixSec(rtcp, timespec->tv_time) ;
}

//! @}
