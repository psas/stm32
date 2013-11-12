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

#include "psas_sdclog.h"

#define         DEBUG_SDLOGEXP

#ifdef DEBUG_SDLOGEXP
#include "usbdetail.h"
BaseSequentialStream    *sdclog   =  (BaseSequentialStream *)&SDU_PSAS;
#define SDLOGEXPDBG(format, ...) chprintf( sdclog, format, ##__VA_ARGS__ )
#else
BaseSequentialStream    *chp   =  (BaseSequentialStream *)&SDU_PSAS;
#define SDLOGEXPDBG(...) do{ } while ( false )
#endif

static const    char*           sdc_log_data_file                = "LOGSMALL.bin";
static const    unsigned        sdlog_thread_sleeptime_ms        = 1234;

static struct dfstate {
    uint32_t            log_sequence;  
    uint32_t            write_errors;

    GENERIC_message     log_data;

    FIL                 DATAFil;

    bool                sd_log_opened;
} datafile_state;


/*! Stack area for the sdlog_thread.  */
WORKING_AREA(wa_sdlog_thread, SDC_THREAD_STACKSIZE_BYTES); 

/*! \brief sdlog thread.  
 *
 * Test logging to microSD card on e407. 
 *
 */
msg_t sdlog_thread(void *p) {
    void * arg __attribute__ ((unused)) = p;
    chRegSetThreadName("sdlog_thread");

#ifdef DEBUG_SDLOGEXP
    chThdSleepMilliseconds(1000);
#endif

    SDLOGEXPDBG("Start sdlog thread\r\n");

    // init structure
    datafile_state.log_sequence  = 0;
    datafile_state.write_errors  = 0;
    datafile_state.sd_log_opened = false;

    sdc_reset_fp_index();

    sdc_init_eod((uint8_t)0xa5);

    // Assert data is halfword aligned
    if(((sizeof(GENERIC_message)*8) % 16) != 0) {
        SDLOGEXPDBG("%s: GENERIC message is not halfword aligned.\r\n", __func__);
        return (SDC_ASSERT_ERROR);
    }

    // Assert we will not overflow Payload
    if(sizeof(MPU9150_read_data) > (sizeof(datafile_state.log_data.data)-1)) {
        SDLOGEXPDBG("%s: DATA size is too large\r\n");
        return (SDC_ASSERT_ERROR);
    }

    Mon 11 November 2013 19:30:48 (PST) break this out for separate function now.
    while(1) {
        uint32_t            bw;
        int                 rc;
        FRESULT             ret;

        if(fs_ready && !datafile_state.sd_log_opened ) {
            // open an existing log file for writing
            ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
            if(ret) { // try again....
                SDLOGEXPDBG("open existing ret: %d\r\n", ret);
                ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_OPEN_EXISTING | FA_WRITE );
            }

            if (ret) {
                SDLOGEXPDBG("failed to open existing %s\r\n",sdc_log_data_file);
                // ok...try creating the file
                ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE   );
                if(ret) {
                    // try again 
                    SDLOGEXPDBG("open new file ret: %d\r\n", ret);
                    ret = f_open(&datafile_state.DATAFil, sdc_log_data_file, FA_CREATE_ALWAYS | FA_WRITE   );
                }
                if (ret) {
                    datafile_state.sd_log_opened = false;
                } else {
                    datafile_state.sd_log_opened = true;
                }
            } else {
                SDLOGEXPDBG("Opened existing file OK.\r\n");
                datafile_state.sd_log_opened = true;
                datafile_state.write_errors  = 0;
            }
        }

        if (fs_ready && datafile_state.sd_log_opened) {
            crc_t          crc16;
            RTCTime        timenow;

            // ID of this message
            strncpy(datafile_state.log_data.mh.ID, mpuid, sizeof(datafile_state.log_data.mh.ID));   // pretend to use mpu sensor for testing

            // index
            datafile_state.log_data.mh.index       = datafile_state.log_sequence++;

            // timestamp
            timenow.h12             = 1;

            rc = psas_rtc_get_unix_time( &RTCD1, &timenow) ;
            if (rc == -1) {
                SDLOGEXPDBG( "%s: psas_rtc time read errors: %d\r\n",__func__, rc);
            }
            datafile_state.log_data.logtime.tv_time = timenow.tv_time;
            datafile_state.log_data.logtime.tv_msec = timenow.tv_msec;
            psas_rtc_to_psas_ts(&datafile_state.log_data.mh.ts, &timenow);

            memcpy(&datafile_state.log_data.data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data) );

            datafile_state.log_data.mh.data_length = sizeof(MPU9150_read_data);

            rc = sdc_write_log_message(&datafile_state.DATAFil, &datafile_state.log_data, &bw) ;
            if(rc != FR_OK ) { ++datafile_state.write_errors; SDLOGEXPDBG("*"); }

            // calc checksum
            crc16                   = crc_init();
            crc16                   = crc_update(crc16, (const unsigned char*) &datafile_state.log_data, sizeof(GENERIC_message));
            crc16                   = crc_finalize(crc16);

            rc = sdc_write_checksum(&datafile_state.DATAFil, &crc16, &bw) ;

            if(rc != FR_OK ) { ++datafile_state.write_errors; SDLOGEXPDBG("+"); }

#ifdef DEBUG_SDLOGEXP
            if((sdc_fp_index - sdc_fp_index_old) > 100000) { 
                if(datafile_state.write_errors !=0) {
                    SDLOGEXPDBG("E%d", datafile_state.write_errors); 
                } else {
                    SDLOGEXPDBG("x");
                }
                sdc_fp_index_old = sdc_fp_index;
            }
#endif

        } else {
            if(datafile_state.sd_log_opened) {
                ret = f_close(&datafile_state.DATAFil);       // might be redundant if card removed....\sa f_sync
                SDLOGEXPDBG( "close file ret: %d\r\n", ret);
                datafile_state.sd_log_opened = false;
            }
        }
        chThdSleepMilliseconds(sdlog_thread_sleeptime_ms);
    }
    return -1;
}

//! @}

