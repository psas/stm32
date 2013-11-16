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
#include "MPL3115A2.h"
#include "ADIS16405.h"

#include "crc_16_reflect.h"

#include "sdcdetail.h"

#include "psas_sdclog.h"

#define         DEBUG_SDCLOG

#ifdef DEBUG_SDCLOG
#include "usbdetail.h"
BaseSequentialStream    *sdclog   =  (BaseSequentialStream *)&SDU_PSAS;
#define SDCLOGDBG(format, ...) chprintf( sdclog, format, ##__VA_ARGS__ )
#else
BaseSequentialStream    *chp   =  (BaseSequentialStream *)&SDU_PSAS;
#define SDCLOGDBG(...) do{ } while ( false )
#endif

enum WHICH_SENSOR { MPU9150=1, MPL3115A2, ADIS16405 };

static const    char*           sdc_log_data_file                = "LOGSMALL.bin";
static const    unsigned        sdlog_thread_sleeptime_ms        = 1234;

static struct dfstate {
    uint32_t            log_sequence;
    uint32_t            write_errors;

    GENERIC_message     log_data;

    FIL                 DATAFil;

    bool                sd_log_opened;
} datafile_state;


static void sdc_log_data(eventid_t id) {
    static const  int32_t      mpu_downsample  = 20;
    static const  int32_t      mpl_downsample  = 20;
    //static const  int32_t      adis_downsample = 20;

    static  int32_t      mpu_count       = 0; 
    static  int32_t      mpl_count       = 0; 
    //static  int32_t      adis_count      = 0; 

    bool                write_log       = false;
    uint32_t            bw;
    int                 rc;
    FRESULT             ret;

    if(fs_ready && !datafile_state.sd_log_opened ) {
        // open an existing log file for writing
        ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_READ | FA_WRITE );
        if(ret) { // try again....
            SDCLOGDBG("open existing failed ret: %d\r\n", ret);
            chThdSleepMilliseconds(500);
            ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_READ | FA_WRITE );
        }

        if (ret) {
            SDCLOGDBG("failed to open existing %s return %d\r\n",sdc_log_data_file, ret);
            // ok...try creating the file
            ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE );
            if(ret) {
                // try again
                SDCLOGDBG("open new file ret: %d\r\n", ret);
                ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE );
            }
            if (ret) {
                datafile_state.sd_log_opened = false;
            } else {
                datafile_state.sd_log_opened = true;
            }
        } else {
            SDC_ERRORCode  seekret;
            SDCLOGDBG("Opened existing file OK.\r\n");
            /* Seek to end of data if first line is good data */
            seekret = sdc_seek_eod(&datafile_state.DATAFil, &sdc_fp_index);
            if(seekret == SDC_OK) {
                sdc_set_fp_index(&datafile_state.DATAFil, sdc_fp_index) ;
            } else {
                sdc_reset_fp_index();
            }
            datafile_state.sd_log_opened = true;
            datafile_state.write_errors  = 0;
        }
    }

    if (fs_ready && datafile_state.sd_log_opened) {
        crc_t          crc16;
        RTCTime        timenow;

        datafile_state.log_data.mh.index        = datafile_state.log_sequence++;

        // timestamp
        timenow.h12                             = 1;
        rc                                      = psas_rtc_get_unix_time( &RTCD1, &timenow) ;
        if (rc == -1) {
            SDCLOGDBG( "%s: psas_rtc time read errors: %d\r\n",__func__, rc);
        }
        datafile_state.log_data.logtime.tv_time = timenow.tv_time;
        datafile_state.log_data.logtime.tv_msec = timenow.tv_msec;
        psas_rtc_to_psas_ts(&datafile_state.log_data.mh.ts, &timenow);

        //SDCLOGDBG("%d ", id);
        switch(id) {
            case MPU9150:
                if(mpu_count++ > mpu_downsample) {
                    SDCLOGDBG("u");
                    strncpy(datafile_state.log_data.mh.ID, mpuid, sizeof(datafile_state.log_data.mh.ID));
                    memcpy(&datafile_state.log_data.data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data) );
                    datafile_state.log_data.mh.data_length = sizeof(MPU9150_read_data);
                    mpu_count = 0;
                    write_log = true;
                }
                break;
            case MPL3115A2:
                if(mpl_count++ > mpl_downsample) {
                    SDCLOGDBG("l");
                    strncpy(datafile_state.log_data.mh.ID, mplid, sizeof(datafile_state.log_data.mh.ID));
                    memcpy(&datafile_state.log_data.data, (void*) &mpl3115a2_current_read, sizeof(MPL3115A2_read_data) );
                    datafile_state.log_data.mh.data_length = sizeof(MPL3115A2_read_data);
                    mpl_count = 0;
                    write_log = true;
                }            
                break;
            case ADIS16405:
                /*
                 *strncpy(datafile_state.log_data.mh.ID, adisid, sizeof(datafile_state.log_data.mh.ID));
                 *memcpy(&datafile_state.log_data.data, (void*) &adis16405_burst_data, sizeof(ADIS16405_burst_data) );
                 *datafile_state.log_data.mh.data_length = sizeof(ADIS16405_burst_data);
                 */
                break;
            default:
                break;
        }

//        if(false) {
        if(write_log) {
            rc = sdc_write_log_message(&datafile_state.DATAFil, &datafile_state.log_data, &bw) ;
            if(rc != FR_OK ) { ++datafile_state.write_errors; SDCLOGDBG("*"); }

            // calc checksum
            crc16                   = crc_init();
            crc16                   = crc_update(crc16, (const unsigned char*) &datafile_state.log_data, sizeof(GENERIC_message));
            crc16                   = crc_finalize(crc16);

            rc = sdc_write_checksum(&datafile_state.DATAFil, &crc16, &bw) ;

            if(rc != FR_OK ) { ++datafile_state.write_errors; SDCLOGDBG("+"); }

#ifdef DEBUG_SDCLOG
            if((sdc_fp_index - sdc_fp_index_old) > 100000) {
                if(datafile_state.write_errors !=0) {
                    SDCLOGDBG("E%d", datafile_state.write_errors);
                } else {
                    SDCLOGDBG("x");
                }
                sdc_fp_index_old = sdc_fp_index;
            }
#endif
            write_log = false;
        }

    } else {
        if(datafile_state.sd_log_opened) {
            ret = f_close(&datafile_state.DATAFil);       // might be redundant if card removed....\sa f_sync
            SDCLOGDBG( "close file ret: %d\r\n", ret);
            datafile_state.sd_log_opened = false;
        }
    }
}

/*! Stack area for the sdlog_thread.  */
WORKING_AREA(wa_sdlog_thread, SDC_THREAD_STACKSIZE_BYTES);

/*! \brief sdlog thread.
 *
 * Test logging to microSD card on e407.
 *
 */
msg_t sdlog_thread(void *p) {
    void * arg __attribute__ ((unused)) = p;
    static const evhandler_t evhndl_sdclog[]  = {
        sdc_log_data,
        sdc_log_data,
        sdc_log_data
    };
    struct EventListener     el0, el1, el2;

    chRegSetThreadName("sdlog_thread");

#ifdef DEBUG_SDCLOG
    chThdSleepMilliseconds(1000);
#endif

    SDCLOGDBG("Start sdlog thread\r\n");

    // init structure
    datafile_state.log_sequence  = 0;
    datafile_state.write_errors  = 0;
    datafile_state.sd_log_opened = false;

    sdc_reset_fp_index();

    sdc_init_eod((uint8_t)0xa5);

    // Assert data is halfword aligned
    if(((sizeof(GENERIC_message)*8) % 16) != 0) {
        SDCLOGDBG("%s: GENERIC message is not halfword aligned.\r\n", __func__);
        return (SDC_ASSERT_ERROR);
    }

    // Assert we will not overflow Payload
    if(  (sizeof(MPU9150_read_data)    > (sizeof(datafile_state.log_data.data)-1)) ||
         (sizeof(MPL3115A2_read_data)  > (sizeof(datafile_state.log_data.data)-1)) ||
         (sizeof(ADIS16405_burst_data) > (sizeof(datafile_state.log_data.data)-1))) {
        SDCLOGDBG("%s: DATA size is too large\r\n");
        return (SDC_ASSERT_ERROR);
    }

    chEvtRegister(&mpl3115a2_data_event        ,   &el0, MPL3115A2);
    chEvtRegister(&adis_spi_burst_data_captured,   &el1, ADIS16405);
    chEvtRegister(&mpu9150_data_event          ,   &el2, MPU9150);

    while(1) {
        chEvtDispatch(evhndl_sdclog, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(50)));
    }
    return -1;
}

//! @}

