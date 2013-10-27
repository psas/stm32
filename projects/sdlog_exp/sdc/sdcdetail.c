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
#include "psas_rtc.h"

#include "MPU9150.h"

#include "crc_16_reflect.h"
#include "sdcdetail.h"

#define         DEBUG_SDC

#ifdef DEBUG_SDC
#include "usbdetail.h"
BaseSequentialStream    *sdcchp   =  (BaseSequentialStream *)&SDU_PSAS;
#define SDCDEBUG(format, ...) chprintf( sdcchp, format, ##__VA_ARGS__ )
DWORD           sdc_fp_index_old                                 = 0;
#else
#define SDCDEBUG(...) do{ } while ( false )
#endif

// end of data marker
static          sdc_eod_marker  sdc_eod;

static const    unsigned        sdlog_thread_sleeptime_ms        = 10;
static const    unsigned        sdc_polling_interval             = 10;
static const    unsigned        sdc_polling_delay                = 10;

static const    char*           sdc_log_data_file                = "LOGSMALL.bin";

static          bool            sd_log_opened                    = false;
static          VirtualTimer    sdc_tmr;

static          unsigned        sdc_debounce_count               = 0;

DWORD           sdc_fp_index                                     = 0;

bool                            fs_ready                         = FALSE;

EventSource                     inserted_event, removed_event;

FATFS                           SDC_FS;

static inline void            sdc_reset_fp_index(void) {
    sdc_fp_index = 0;
}

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
        err = f_mount(0, &SDC_FS);
        if (err != FR_OK) {
            sdcDisconnect(&SDCD1);
            return;
        }
    }
    sdc_reset_fp_index();
    fs_ready = TRUE;
}

/*!
 *  \brief Card removal event.
 */
void RemoveHandler(eventid_t id) {
    (void)id;
    bool_t ret;
    ret = sdcDisconnect(&SDCD1);
    if(ret) {
        SDCDEBUG("sdcDiscon fail\r\n");   // this happens a lot!
        ret = sdcDisconnect(&SDCD1);
        if(ret) {
            SDCDEBUG("sdcDiscon fail2\r\n");
        }
    }
    sdc_reset_fp_index();
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


void sdc_set_fp_index(FIL* DATAFil, DWORD ofs) {
    f_lseek(DATAFil, ofs);
    sdc_fp_index      = ofs;
}

FRESULT   sdc_write_checksum(FIL* DATAFil, const crc_t* crcd, unsigned int* bw) {
    FRESULT rc;

    if((crcd==NULL) || (bw == NULL)) {
        return SDC_NULL_PARAMETER_ERROR;
    }
    rc = f_write(DATAFil, (const void *)(crcd), sizeof(crc_t), bw);
    if (rc)  {
        SDCDEBUG("f_write ckdata error: %d\r\n", rc) ;
        return rc;
    }
    sdc_fp_index += *bw;

    rc = f_write(DATAFil, (const void *)(&sdc_eod), sizeof(sdc_eod_marker), bw);
    if (rc)  {
        SDCDEBUG("f_write eod error: %d\r\n", rc) ;
        return rc;
    }
    sdc_fp_index += *bw;

    rc = f_sync(DATAFil);
    if (rc)  {
        SDCDEBUG("f_sync error: %d\r\n", rc);
        return rc;
    }

    return FR_OK;
}

FRESULT   sdc_write_log_message(FIL* DATAFil, GENERIC_message* d, unsigned int* bw) {
    FRESULT rc;

    if((d==NULL) || (bw == NULL)) {
        return SDC_NULL_PARAMETER_ERROR;
    }

    rc = f_write(DATAFil, (const void *)(d), sizeof(GENERIC_message), bw);
    if (rc)  {
        SDCDEBUG("f_write log error:\r\n") ;
        return rc;
    }
    sdc_fp_index += *bw;

    rc = f_sync(DATAFil);
    if (rc)  {
        SDCDEBUG("%s: f_sync error: %d\r\n", __func__, rc);
        return rc;
    }
    return FR_OK;
}

/*!
 * Stack area for the sdlog_thread.
 */
WORKING_AREA(wa_sdlog_thread, SDC_THREAD_STACKSIZE_BYTES);

/*! \brief sdlog  thread.
 *
 * \param p
 * \return -1: generic error
 *         -2: unable to open file
 */
msg_t sdlog_thread(void *p) {
    void * arg __attribute__ ((unused)) = p;

    GENERIC_message   log_data;
    int               log_index = 0;
    int               rc;
    unsigned int       i;

    FRESULT           ret;
    FIL               DATAFil;

    //     const char adisid[(sizeof("ADIS")-1)] = "ADIS";
    //     const char mplid[(sizeof("MPL3")-1)]  = "MPL3";

    unsigned int      bw = 0;
    uint32_t          write_errors     = 0;

    chThdSleepMilliseconds(1000);

    chRegSetThreadName("sdlog_thread");

    SDCDEBUG("Start sdlog thread\r\n");

    // init index
    sdc_reset_fp_index();

    // init fiducial markers.
    for(i=0; i<sizeof(sdc_eod_marker) ; ++i) {
        sdc_eod.sdc_eodmarks[i] = 0xa5;
    }

    // Assert we will end up on 16bit boundary
    if((sizeof(GENERIC_message) % 16) != 0) {
        SDCDEBUG("%s: GENERIC message is not halfword aligned.\r\n", __func__);
        return (SDC_ASSERT_ERROR);
    }

    // Assert we will not overflow Payload
    if(sizeof(MPU9150_read_data) > (sizeof(log_data.data)-1)) {
        SDCDEBUG("%s: DATA size is too large\r\n");
        return (SDC_ASSERT_ERROR);
    }

    while(1) {
        if(fs_ready && (!sd_log_opened) ) {
            // open an existing log file for writing
            ret = f_open(&DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
            if(ret) { // try again....
                SDCDEBUG("open ret: %d\r\n", ret);
                ret = f_open(&DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
            }

            if (ret) {
                SDCDEBUG("failed to open existing %s\r\n",sdc_log_data_file);
                // ok...try creating the file
                SDCDEBUG("open new\t");
                ret = f_open(&DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE   );
                if (ret) {
                    sd_log_opened = false;
                } else {
                    sd_log_opened = true;
                }
            } else {
                SDCDEBUG( "Opened existing file OK.\r\n");
                sd_log_opened = true;
            }
        }

        if (fs_ready && sd_log_opened) {
            crc_t          crc16;
            RTCTime        timenow;

            // ID of this message
            strncpy(log_data.mh.ID, mpuid, sizeof(log_data.mh.ID));   // pretend to use mpu sensor for testing

            // index
            log_data.mh.index       = log_index++;

            // timestamp
            timenow.h12             = 1;
            rc = psas_rtc_get_unix_time( &RTCD1, &timenow) ;
            if (rc == -1) {
                SDCDEBUG( "%s: psas_rtc time read errors: %d\r\n",__func__, rc);
                continue;
            }
            psas_rtc_to_psas_ts(&log_data.mh.ts, &timenow);

            // payload
            memcpy(&log_data.data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data) );

            // length of payload
            log_data.mh.data_length = sizeof(MPU9150_read_data);

            // write log message
            rc = sdc_write_log_message(&DATAFil, &log_data, &bw) ;
            if(rc != FR_OK ) { 
                ++write_errors; 
                SDCDEBUG("+"); 
                }

            // calc checksum
            crc16                   = crc_init();
            crc16                   = crc_update(crc16, (const unsigned char*) &log_data, sizeof(GENERIC_message));
            crc16                   = crc_finalize(crc16);

            // write checksum and fiducials.
            rc = sdc_write_checksum(&DATAFil, &crc16, &bw) ;
            if(rc != FR_OK ) { ++write_errors; SDCDEBUG("+"); }

#ifdef DEBUG_SDC
            if((sdc_fp_index - sdc_fp_index_old) > 100000){ 
                SDCDEBUG("x"); 
                sdc_fp_index_old = sdc_fp_index;
            }
#endif

        } else {
            if(sd_log_opened) {
                ret = f_close(&DATAFil);       // might be redundant if card removed....
                //SDCDEBUG( "close file ret: %d\r\n", ret);
                sd_log_opened = false;
            }
        }
        chThdSleepMilliseconds(sdlog_thread_sleeptime_ms);
    }
    return -1;
}

//! @}

