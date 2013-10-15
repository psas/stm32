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
#include "chrtclib.h"

#include "ff.h"
#include "usbdetail.h"
#include "psas_rtc.h"

#include "sdcdetail.h"

static const    unsigned        sdlog_thread_sleeptime_ms      = 1000;
static const    unsigned        sdc_polling_interval           = 10;
static const    unsigned        sdc_polling_delay              = 10;

// static const    char*           sdc_log_data_file               = "data_log.txt";
// static const    char*           sdc_log_data_file               = "LOGFILEFOURGB.bin";
static const    char*           sdc_log_data_file               = "LOGSMALL.bin";

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

    /* generate a mailbox event here
    */

    /*
       test event message system
       */


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
                SDCDEBUG("%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}

/* Manipulate the file pointer index */
void sdc_read_fp_index() {
    /* not implemented */
}

void sdc_write_fp_index() {
    /* not implemented */
}

void sdc_set_fp_index() {
    /* not implemented */
}

void sdc_reset_fp_index() {
    /* not implemented */
}

static int sdc_write_log_message(FIL* DATAFil, GENERIC_message* d, unsigned int* bw) {

    if((d==NULL) || (bw == NULL)) {
        return SDC_NULL_PARAMETER_ERROR;
    }
    SDCDEBUG("%d\r\n", d->message_head.psas_time_ns);

    rc = f_write(DATAFil, (const void *)(d), sizeof(GENERIC_message), bw);
    if (rc)  {
        SDCDEBUG(chp, "%s: f_write error: %d\r\n", __func__, rc);
        return SDC_FWRITE_ERROR1;
    }

    rc = f_sync(DATAFil);
    if (rc)  {
        SDCDEBUG(chp, "%s: f_sync error: %d\r\n", __func__, rc);
        return SDC_FSYNC_ERROR;
    }
    return SDC_OK;
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
    GENERIC_message   log_data;
    FRESULT           ret;
    RTCTime           psas_time_ns;
    FIL               DATAFil;
    SDC_ERRORCode     sdc_ret;

    unsigned int      bw;
    uint32_t          index            = 0;
    uint32_t          write_errors     = 0;
    uint32_t          sync_errors      = 0;

    chRegSetThreadName("sdlog_thread");

    // write First Line with EVENT 'opened logfile' 'version'
    //    set up events
    //      mpu  newdata
    //      mpl  newdata
    //      adis newdata

    SDCDEBUG(chp, "Start sdlog thread\r\n");
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
                SDCDEBUG(chp, "open new\t");
                rc = f_open(&DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE   );
                if (rc) {
                    sd_log_opened = false;
                } else {
                    sd_log_opened = true;
                    SDCDEBUG(chp, "opened new\t");
                    //    				rc = f_write(&DATAFil, "New File Started\n", sizeof("New File Started\n"), &bw);
                    //    				SDCDEBUG(chp, "write new file %d bytes\r\n", bw);
                    //    				rc = f_sync(&DATAFil);
                    //    				if (rc)  {
                    //    					SDCDEBUG(chp, "write new file failed\r\n");
                    //    				}
                    //SDCDEBUG(chp, "write new file %d bytes\r\n", bw);
                    //rc = f_close(&DATAFil);
                    //	rc = f_open(&DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
                    //	if (rc) {
                    //		SDCDEBUG(chp, "reopen fail\r\n");
                    //		sd_log_opened = false;
                    //	}
                }
            } else {
                SDCDEBUG(chp, "open existing file ok\r\n");
                sd_log_opened = true;
            }
            //    		if(sd_log_opened) {
            //    			/* Move to end of the file to append data */
            //    			rc = f_lseek(&DATAFil, f_size(&DATAFil));
            //    			if (rc) {
            //    				SDCDEBUG(chp, "seek fail\r\n");
            //    				sd_log_opened = false;
            //    			}
            //    		}
        }

        if (fs_ready && sd_log_opened) {
            RTCTime        timenow;
            log_data.mh.ID       = {'A','D','I','S'};
            timenow.h12          = 1;
            psas_rtc_lld_get_time(&RTCD1, &timenow);
            psas_rtc_to_psas_ns(&log_data.mh.psas_time_ns, &timenow);
            sdc_ret = sdc_write_log_message(&DATAFil, &log_data, &bw) ;
            if(sdc_ret == -1 ) { ++write_errors; }
            if(sdc_ret == -2 ) { ++sync_errors;  }
            SDCDEBUG(chp, "write/sync errors: %d/%d\r\n", write_errors, sync_errors);
        } else {
            if(sd_log_opened) {
                SDCDEBUG(chp, "close file\r\n");
                f_close(&DATAFil);       // might be redundant if card removed....
                sd_log_opened = false;
            }
        }
        chThdSleepMilliseconds(sdlog_thread_sleeptime_ms);
    }
    return -1;
}

//! @}

