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
 * \defgroup psas_rtc Non standard PSAS RTC
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
#include "ch.h"
#include "hal.h"
#include "rtc.h"

#include "stm32f4xx.h"

#include "chrtclib.h"
#include "psas_rtc.h"

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

/*!
 * \brief   Finalizing of configuration procedure.
 */
#define psas_rtc_lld_exit_init() {RTCD1.id_rtc->ISR &= ~RTC_ISR_INIT;}

psas_rtc_state     psas_rtc_s = {false};


/*!
 * \brief   Enable access to registers.
 *
 * This application will use the subsecond register
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

#if PSAS_RTC_COE_DEBUG
    RTCD1.id_rtc->CR   |= RTC_CR_COE;
    RTCD1.id_rtc->CR   |= RTC_CR_COSEL;
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


/*!
 * \brief   Get current time.
 *
 * \param[in] rtcp      pointer to RTC driver structure
 * \param[out] timespec pointer to a @p RTCTime structure
 */
void psas_rtc_lld_get_time( RTCDriver *rtcp, RTCTime *timespec) {
    (void)rtcp;

    if(!psas_rtc_s.initialized) {
        return;
    }

    psas_rtc_lld_apb1_sync();

#if STM32_RTC_HAS_SUBSECONDS
    timespec->tv_msec =1000000 * ((1.0 * (RTCD1.id_rtc->PRER & 0x7FFF) - RTCD1.id_rtc->SSR) /
            ((RTCD1.id_rtc->PRER & 0x7FFF) + 1));
#endif /* STM32_RTC_HAS_SUBSECONDS */
    timespec->tv_time = RTCD1.id_rtc->TR;
    timespec->tv_date = RTCD1.id_rtc->DR;

}

/*!
 * \brief   Get current unix time and microseconds (us).
 * \param[in] rtcp  pointer to RTC driver structure
 * \param[out] s    pointer to a time_t
 * \param[out] us   pointer to a uint32_t
 */
void psas_rtc_lld_get_s_and_us(RTCDriver* rtcp, time_t* s, uint32_t* us) {
  RTCTime received_at_rtc;
  struct tm received_at_tm;
  time_t received_at_unix;

  psas_rtc_lld_get_time(rtcp, &received_at_rtc);
  psas_stm32_rtc_bcd2tm(&received_at_tm, &received_at_rtc);
  received_at_unix = mktime(&received_at_tm);

  *s = received_at_unix;
  *us = received_at_rtc.tv_msec;
}


//! @}
