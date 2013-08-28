/*! \file sdcdetail.c
 *  SD Card
 */

/*!
 * \defgroup sdcdetail SDC Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"
#include "usbdetail.h"
#include "psas_rtc.h"
#include "sdcdetail.h"

#include "chrtclib.h"
#define         DEBUG_SDC                                     0

static const    unsigned        sdlog_thread_sleeptime_ms      = 1000;
static const    unsigned        sdc_polling_interval           = 10;
static const    unsigned        sdc_polling_delay              = 10;

static const    char*           sdc_log_data_file               = "data_log.txt";

static          bool             sd_log_opened                   = false;
static          VirtualTimer     sdc_tmr;

static          unsigned        sdc_debounce_count             = 0;

bool                              fs_ready                       = FALSE;

EventSource                       inserted_event, removed_event;

FATFS                             SDC_FS;

/*!
 * @brief   Insertion monitor timer callback function.
 *
 * @param[in] p         pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void sdc_tmrfunc(void *p) {
    BaseBlockDevice *bbdp = p;

    chSysLockFromIsr();
    if (sdc_debounce_count > 0) {
        if (blkIsInserted(bbdp)) {
            if (--sdc_debounce_count == 0) {
                chEvtBroadcastI(&inserted_event);
            }
        }
        else
            sdc_debounce_count = sdc_polling_interval;
    }
    else {
        if (!blkIsInserted(bbdp)) {
            sdc_debounce_count = sdc_polling_interval;
            chEvtBroadcastI(&removed_event);
        }
    }
    chVTSetI(&sdc_tmr, MS2ST(sdc_polling_delay), sdc_tmrfunc, bbdp);
    chSysUnlockFromIsr();
}

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
void sdc_tmr_init(void *p) {

    chEvtInit(&inserted_event);
    chEvtInit(&removed_event);
    chSysLock();
    sdc_debounce_count = sdc_polling_interval;
    chVTSetI(&sdc_tmr, MS2ST(sdc_polling_delay), sdc_tmrfunc, p);
    chSysUnlock();
}

/*!
 * \brief Card insertion event.
 */
void InsertHandler(eventid_t id) {
    FRESULT err;

    (void)id;

    /*
     * On insertion SDC initialization and FS mount.
     */
    if (sdcConnect(&SDCD1)) {
        if(sdcConnect(&SDCD1)) {   // why does it often fail the first time but not the second?
            return;
        }
    }

    err = f_mount(0, &SDC_FS);
    if (err != FR_OK) {
        sdcDisconnect(&SDCD1);
        return;
    }
    fs_ready = TRUE;
}

/*!
 *  \brief Card removal event.
 */
void RemoveHandler(eventid_t id) {
    (void)id;
    sdcDisconnect(&SDCD1);
    fs_ready = FALSE;
}


FRESULT sdc_scan_files(BaseSequentialStream *chp, char *path) {
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;

#if _USE_LFN
    fno.lfname = 0;
    fno.lfsize = 0;
#endif
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            if (fno.fname[0] == '.')
                continue;
            fn = fno.fname;
            if (fno.fattrib & AM_DIR) {
                path[i++] = '/';
                strcpy(&path[i], fn);
                res = sdc_scan_files(chp, path);
                if (res != FR_OK)
                    break;
                path[--i] = 0;
            }
            else {
                chprintf(chp, "%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}


/*! \brief sdlog  thread.
 *
 * \return -1: generic error
 *         -2: unable to open file
 *         -3: Null data structure
 *
 *  \todo return typedef or enum...
 */

static int write_log_data(FIL* DATAFil, Logdata* d, unsigned int* bw) {
    int              rc = 0;
    BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;

    if((d==NULL) || (bw == NULL)) {
        return -3;
    }
#if DEBUG_SDC
    chprintf(chp, "%d\r\n", d->timespec.tv_time);
    chprintf(chp, "sizeof logdata:\t%d\r\n", sizeof(Logdata));
#endif
    //rc = f_write(DATAFil, "More Stuff\r\n", sizeof("More Stuff\r\n")-1, bw);
    rc = f_write(DATAFil, (const void *)(d), sizeof(Logdata), bw);
    if (rc)  {
        chprintf(chp, "%s: f_write error: %d\r\n", __func__, rc);
        return -1;
    }

    rc = f_sync(DATAFil);
    if (rc)  {
    	chprintf(chp, "%s: f_sync error: %d\r\n", __func__, rc);
    	return -2;
    }
    return 0;
}

/*!
 * Stack area for the sdlog_thread.
 */
WORKING_AREA(wa_sdlog_thread, SDLOG_THREAD_STACKSIZE_BYTES);
//
//struct RTCTime {
//  /**
//   * @brief RTC date register in STM32 BCD format.
//   */
//  uint32_t tv_date;
//  /**
//   * @brief RTC time register in STM32 BCD format.
//   */
//  uint32_t tv_time;
//  /**
//   * @brief Set this to TRUE to use 12 hour notation.
//   */
//  bool_t h12;
//  /**
//   * @brief Fractional part of time.
//   */
//#if STM32_RTC_HAS_SUBSECONDS
//  uint32_t tv_msec;
//#endif
//};




/*! \brief sdlog  thread.
 *
 * \param p
 * \return -1: generic error
 *         -2: unable to open file
 */
msg_t sdlog_thread(void *p) {
    void * arg __attribute__ ((unused)) = p;
    Logdata           log_data;
    FRESULT           rc;
    FIL               DATAFil;
    int result;
    unsigned int     bw;
    uint32_t          index            = 0;
    uint32_t          write_errors     = 0;
    uint32_t          sync_errors      = 0;

    BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;
    chRegSetThreadName("sdlog_thread");

//    set up events
//      mpu  newdata
//      mpl  newdata
//      adis newdata

    chprintf(chp, "Start sdlog thread\r\n");
    while(1) {

    	if(fs_ready && (!sd_log_opened) ) {
    		// open an existing log file for writing
    		/*!
    		 *  There is a proposal to pre-allocate a large file
    		 *  and open it, instead of creating a file. The theory
    		 *  is that in the case of a system failure, the file
    		 *  will be able to be re-opened even if garbage was written
    		 *  to it. Should the system fail during a block allocation
    		 *  or similar function, the file (or card) may be left in an
    		 *  unreadable state. In the preallocated file proposal, only
    		 *  f_open and f_close functions are applied.
    		 *
    		 *  \todo There is need for some experimenting here.
    		 *
    		 */

    		rc = f_open(&DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
    		if (rc) {
    			// ok...try creating the file
    			chprintf(chp, "open new\t");
    			rc = f_open(&DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE   );
    			if (rc) {
    				sd_log_opened = false;
    			} else {
    				sd_log_opened = true;
    				chprintf(chp, "opened new\t");
//    				rc = f_write(&DATAFil, "New File Started\n", sizeof("New File Started\n"), &bw);
//    				chprintf(chp, "write new file %d bytes\r\n", bw);
//    				rc = f_sync(&DATAFil);
//    				if (rc)  {
//    					chprintf(chp, "write new file failed\r\n");
//    				}
    				//chprintf(chp, "write new file %d bytes\r\n", bw);
    				//rc = f_close(&DATAFil);
    			//	rc = f_open(&DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
    			//	if (rc) {
    			//		chprintf(chp, "reopen fail\r\n");
    			//		sd_log_opened = false;
    			//	}
    			}
    		} else {
    			chprintf(chp, "open existing file ok\r\n");
    			sd_log_opened = true;
    		}
//    		if(sd_log_opened) {
//    			/* Move to end of the file to append data */
//    			rc = f_lseek(&DATAFil, f_size(&DATAFil));
//    			if (rc) {
//    				chprintf(chp, "seek fail\r\n");
//    				sd_log_opened = false;
//    			}
//    		}
    	}

        if (fs_ready && sd_log_opened) {
            log_data.index        = index++;
            log_data.timespec.h12 = 1;
            psas_rtc_lld_get_time(&RTCD1, &log_data.timespec);

            result = write_log_data(&DATAFil, &log_data, &bw);
            if(result == -1 ) { ++write_errors; }
            if(result == -2) { ++sync_errors; }
#if DEBUG_SDC
            chprintf(chp, "write/sync errors: %d/%d\r\n", write_errors, sync_errors);
#endif
        } else {
        	if(sd_log_opened) {
        		chprintf(chp, "close file\r\n");
        		f_close(&DATAFil);       // might be redundant if card removed....
        		sd_log_opened = false;
        	}
        }
        chThdSleepMilliseconds(sdlog_thread_sleeptime_ms);
    }
    return -1;
}

//! @}

