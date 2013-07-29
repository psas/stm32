/*! \file cmddetail.c
 *
 */

/*!
 * \defgroup cmddetail Command Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "rtc.h"
#include "chprintf.h"
#include "mac.h"
#include "stm32f4xx.h"

#include "chrtclib.h"

#include "cmddetail.h"

#define 		DEBUG_PHY 			0


static time_t unix_time;

/**
 * @brief   Wait for synchronization of RTC registers with APB1 bus.
 * @details This function must be invoked before trying to read RTC registers.
 *
 * @notapi
 */
#define psas_rtc_lld_apb1_sync() {while ((RTCD1.id_rtc->ISR & RTC_ISR_RSF) == 0);}

/**
 * @brief   Converts from STM32 BCD to canonicalized time format.
 *
 * @param[out] timp     pointer to a @p tm structure as defined in time.h
 * @param[in] timespec  pointer to a @p RTCTime structure
 *
 * @notapi
 */
static void stm32_rtc_bcd2tm(struct tm *timp, RTCTime *timespec) {
  uint32_t tv_time = timespec->tv_time;
  uint32_t tv_date = timespec->tv_date;

#if CH_DBG_ENABLE_CHECKS
  timp->tm_isdst = 0;
  timp->tm_wday  = 0;
  timp->tm_mday  = 0;
  timp->tm_yday  = 0;
  timp->tm_mon   = 0;
  timp->tm_year  = 0;
  timp->tm_sec   = 0;
  timp->tm_min   = 0;
  timp->tm_hour  = 0;
#endif

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


/**
 * @brief   Get current time.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timespec pointer to a @p RTCTime structure
 *
 * @api
 */
static void psas_rtc_lld_get_time( RTCDriver *rtcp, RTCTime *timespec) {
    (void)rtcp;

    psas_rtc_lld_apb1_sync();

#if STM32_RTC_HAS_SUBSECONDS
    timespec->tv_msec =1000000 * ((1.0 * (RTCD1.id_rtc->PRER & 0x7FFF) - RTCD1.id_rtc->SSR) /
            ((RTCD1.id_rtc->PRER & 0x7FFF) + 1));
#endif /* STM32_RTC_HAS_SUBSECONDS */
    timespec->tv_time = RTCD1.id_rtc->TR;
    timespec->tv_date = RTCD1.id_rtc->DR;


}

void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)argv;
  struct tm timp;
  RTCTime   psas_time;

  if (argc == 0) {
      chprintf(chp, "PRER:         0x%x\r\n", RTCD1.id_rtc->PRER);
	  chprintf(chp, "CR:           0x%x\r\n", RTCD1.id_rtc->CR);
	  chprintf(chp, "RCC->CSR:     0x%x\r\n", RCC->CSR);
	  chprintf(chp, "[4:0]RCC->CFGR:    0x%x\r\n", RCC->CFGR);
	  chprintf(chp, "[1:0]RCC->BDCR:    0x%x\r\n", RCC->BDCR);
	  chprintf(chp, "STM32_RTCCLK: 0x%x\r\n", STM32_RTCCLK);
    goto ERROR;
  }


  if ((argc == 1) && (strcmp(argv[0], "get") == 0)){
      //int i;
      //unix_time = rtcGetTimeUnixSec(&RTCD1);
      //for (i=0; i<5000; ++i) {
      //  chThdSleepMilliseconds(1);

      psas_rtc_lld_get_time(&RTCD1, &psas_time);
      stm32_rtc_bcd2tm(&timp, &psas_time);

      unix_time = mktime(&timp);

      if (unix_time == -1){
          chprintf(chp, "incorrect time in RTC cell\r\n");
      }
      else{
          chprintf(chp, "%Ds %Dus %s",unix_time, psas_time.tv_msec, " - unix time\r\n");
          //              rtcGetTimeTm(&RTCD1, &timp);
          //              chprintf(chp, "%s%s",asctime(&timp)," - formatted time string\r\n");
      }
      // }
      return;
  }

  if ((argc == 2) && (strcmp(argv[0], "set") == 0)){
    unix_time = atol(argv[1]);
    if (unix_time > 0){
      rtcSetTimeUnixSec(&RTCD1, unix_time);
      return;
    }
    else{
      goto ERROR;
    }
  }
  else{
    goto ERROR;
  }

ERROR:
  chprintf(chp, "Usage: date get\r\n");
  chprintf(chp, "       date set N\r\n");
  chprintf(chp, "where N is time in seconds sins Unix epoch\r\n");
  chprintf(chp, "you can get current N value from unix console by the command\r\n");
  chprintf(chp, "%s", "date +\%s\r\n");
  return;
}

/*! \brief Show memory usage
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

/*! \brief Show running threads
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
	static const char *states[] = {THD_STATE_NAMES};
	Thread *tp;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: threads\r\n");
		return;
	}
	chprintf(chp, "addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");
	tp = chRegFirstThread();
	do {
		chprintf(chp, "%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n",
				(uint32_t)tp, (uint32_t)tp->p_ctx.r13,
				(uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
				states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
		tp = chRegNextThread(tp);
	} while (tp != NULL);
}


/*! \brief   Read a PHY register.
 *
 */
static uint32_t mii_read(MACDriver *macp, uint32_t reg) {

 ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR | ETH_MACMIIAR_MB;
 while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
   ;
 return ETH->MACMIIDR;
}

#if DEBUG_PHY
/*! \brief   Writes a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 * @param[in] value     new register value
 */
static void mii_write(MACDriver *macp, uint32_t reg, uint32_t value) {

  ETH->MACMIIDR = value;
  ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR |
                  ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;
  while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
	  ;
}
#endif

void cmd_phy(BaseSequentialStream *chp, int argc, char *argv[]) {
	uint32_t phy_val     = 0;
	uint32_t reg_to_ping = 0;

	//uint32_t bmcr_val = 0;

	if (argc != 1) {
		chprintf(chp, "Usage: phy reg(decimal)\r\n");
		return;
	}

//	bmcr_val = mii_read(&ETHD1, MII_BMCR);
//
//	mii_write(&ETHD1, MII_BMCR, (bmcr_val & ~(1<<12)) );
//
//	bmcr_val = mii_read(&ETHD1, MII_BMCR);
//
//	mii_write(&ETHD1, 0x1f,( bmcr_val | 1<<13));

	reg_to_ping = atoi(argv[0]);
	phy_val = mii_read(&ETHD1, reg_to_ping);
	chprintf(chp, "phy reg 0x%x value:\t0x%x\n\r", reg_to_ping, phy_val);

}
//! @}
