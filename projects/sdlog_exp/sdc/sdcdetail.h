/*! \file sdcdetail.h
 *
 */

#ifndef _SDCDETAIL_H
#define _SDCDETAIL_H

/*!
 * \addtogroup sdcdetail
 * @{
 */
#include "ch.h"
#include "hal.h"
#include "ff.h"

#include "psas_rtc.h"
#include "usbdetail.h"

#include "crc_16_reflect.h"

#ifdef __cplusplus
extern "C" {
#endif

#define         SDC_THREAD_STACKSIZE_BYTES                  2048

/* A 162 byte message written at 1000hz will use 4GB in about 6.5 Hours */
#define         SDC_MAX_PAYLOAD_BYTES                       150
#define         SDC_NUM_ID_CHARS                            4

    extern          bool        fs_ready;
    extern          FATFS       SDC_FS;

    extern          EventSource inserted_event;
    extern          EventSource removed_event;
    extern          WORKING_AREA(wa_sdlog_thread, SDC_THREAD_STACKSIZE_BYTES);

    typedef         uint8_t     Payload;

    // create end of data fiducials...slightly larger than log entry
    typedef struct sdc_eod_marker {
        // GENERIC_message + checksum + eod_marker
        uint8_t sdc_eodmarks[SDC_MAX_PAYLOAD_BYTES+13+2+2];
    } sdc_eod_marker;

    typedef enum SDC_ERRORCode {
        SDC_OK                   = 0,
        SDC_NULL_PARAMETER_ERROR = -1,
        SDC_FSYNC_ERROR          = -2,
        SDC_FWRITE_ERROR         = -3,
        SDC_ASSERT_ERROR         = -4,
        SDC_UNKNOWN_ERROR        = -99
    } SDC_ERRORCode;

    struct Message_head {
        char                 ID[SDC_NUM_ID_CHARS]; // This must be first part of data. Reserved value: 0xa5a5
        uint32_t             index;
        psas_timespec        ts;
        uint16_t             data_length;
    } __attribute__((packed));
    typedef struct Message_head Message_head;

    struct GENERIC_message {
        Message_head         mh;
        Payload              data[SDC_MAX_PAYLOAD_BYTES];
        uint8_t              align[10];//align to halfword boundary
    } __attribute__((packed));
    typedef struct GENERIC_message GENERIC_message;

    void            InsertHandler(eventid_t id) ;
    void            RemoveHandler(eventid_t id) ;
    void            sdc_tmr_init(void *p) ;
    void            sdc_set_fp_index(FIL* DATAFil, DWORD ofs) ;

    FRESULT         sdc_write_checksum(FIL* DATAFil, const crc_t* d, unsigned int* bw) ;
    FRESULT         sdc_write_log_message(FIL* DATAFil, GENERIC_message* d, unsigned int* bw) ;
    FRESULT         sdc_scan_files(BaseSequentialStream *chp, char *path) ;

    msg_t           sdlog_thread(void *p) ;

#ifdef __cplusplus
}
#endif
//! @}

#endif

