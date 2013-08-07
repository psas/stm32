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

#include "psas_rtc.h"
#include "sdcdetail.h"
static const    unsigned        sdc_polling_interval           = 10;
static const    unsigned        sdc_polling_delay              = 10;
static const    char*           sdc_log_data_file              = "data_log.bin";


static          VirtualTimer    sdc_tmr;

static          unsigned        sdc_debounce_count             = 0;

bool                            fs_ready                       = FALSE;

EventSource                     inserted_event, removed_event;

FATFS                           SDC_FS;

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

/*!
 * Stack area for the sdlog_thread.
 */
WORKING_AREA(wa_sdlog_thread, SDLOG_THREAD_STACK_SIZE);

/*! \brief sdlog  thread.
 *
 * \return -1: generic error
 *         -2: unable to open file
 */

/*! \brief sdlog  thread.
 *
 * \param p
 * \return -1: generic error
 *         -2: unable to open file
 */
msg_t sdlog_thread(void *p) {
    void * arg __attribute__ ((unused)) = p;

    FRESULT           rc;
    FIL               DATAFil;
    bool              sd_log_opened = false;
    Logdata           log_data;
    uint32_t          index         = 0;

    RTCTime           psas_time;

    chRegSetThreadName("sdlog_thread");

//    set up events
//      mpu  newdata
//      mpl  newdata
//      adis newdata

    // if card inserted
    if(fs_ready) {

    }
    while(1) {
        // if card still inserted
        if(fs_ready && (!sd_log_opened) ) {
            // open an existing log file for writing
            rc = f_open(&DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
            if (rc) {
                // try creating the file
                rc = f_open(&DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE   );
                if (rc) {
                    sd_log_opened = true;
                }
            }

            log_data.index = index++;
            psas_rtc_lld_get_time(&RTCD1, &psas_time);
            write_log_data(&log_data) ;

            // if file open
            // get the time


            // write it to sd


            // wait 1mS
        } else if (fs_ready && sd_log_opened) {
            sd_log_opened = false;
            // wait 1mS
        } else {
            sd_log_opened = false;
            // wait 1mS
        }

    }
    return -1;
}

//! @}

