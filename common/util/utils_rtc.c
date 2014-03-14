#include <time.h>
#include "utils_rtc.h"

uint64_t rtc_to_ns(RTCTime * timespec){
    struct tm timp;
    uint32_t tv_time = timespec->tv_time;
    uint32_t tv_date = timespec->tv_date;
    uint32_t tv_msec = timespec->tv_msec;
    time_t unix_time;

    timp.tm_sec  =  (tv_time & RTC_TR_SU) >> RTC_TR_SU_OFFSET;
    timp.tm_sec  += ((tv_time & RTC_TR_ST) >> RTC_TR_ST_OFFSET) * 10;

    timp.tm_min  =  (tv_time & RTC_TR_MNU) >> RTC_TR_MNU_OFFSET;
    timp.tm_min  += ((tv_time & RTC_TR_MNT) >> RTC_TR_MNT_OFFSET) * 10;

    timp.tm_hour =  (tv_time & RTC_TR_HU) >> RTC_TR_HU_OFFSET;
    timp.tm_hour += ((tv_time & RTC_TR_HT) >> RTC_TR_HT_OFFSET) * 10;
    timp.tm_hour += 12 * ((tv_time & RTC_TR_PM) >> RTC_TR_PM_OFFSET);

    timp.tm_isdst = -1; /* fixme: 26.5.3 Daylight Savings Time, stm32 ref man*/

    timp.tm_mday =  (tv_date & RTC_DR_DU) >> RTC_DR_DU_OFFSET;
    timp.tm_mday += ((tv_date & RTC_DR_DT) >> RTC_DR_DT_OFFSET) * 10;

    timp.tm_wday = (tv_date & RTC_DR_WDU) >> RTC_DR_WDU_OFFSET;
    if (timp.tm_wday == 7)
        timp.tm_wday = 0;

    timp.tm_mon  =  (tv_date & RTC_DR_MU) >> RTC_DR_MU_OFFSET;
    timp.tm_mon  += ((tv_date & RTC_DR_MT) >> RTC_DR_MT_OFFSET) * 10;
    timp.tm_mon  -= 1;

    timp.tm_year =  (tv_date & RTC_DR_YU) >> RTC_DR_YU_OFFSET;
    timp.tm_year += ((tv_date & RTC_DR_YT) >> RTC_DR_YT_OFFSET) * 10;
    timp.tm_year += 2000 - 1900;

    unix_time = mktime(&timp);

    uint64_t time_ns = (uint64_t)unix_time * 1000000000 + (uint64_t)tv_msec * 1000000;
    return time_ns;
}
