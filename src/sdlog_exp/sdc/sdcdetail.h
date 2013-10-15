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

#ifdef __cplusplus
extern "C" {
#endif

#define         DEBUG_SDC

#ifdef DEBUG_SDC
    BaseSequentialStream    *chp   =  (BaseSequentialStream *)&SDU_PSAS;
#endif

#ifdef DEBUG_SDC
    #define SDCDEBUG(format, ...) chprintf( chp, format, ##__VA_ARGS__ )
#else
    #define SDCDEBUG(...) do{ } while ( false )
#endif

#define         SDC_THREAD_STACKSIZE_BYTES                  1024

/* A 162 byte message written at 1000hz will use 4GB in about 6.5 Hours */
#define         SDC_MAX_PAYLOAD_BYTES                       150

    extern          bool        fs_ready;
    extern          FATFS       SDC_FS;

    extern          EventSource inserted_event;
    extern          EventSource removed_event;
    extern          WORKING_AREA(wa_sdlog_thread, SDC_THREAD_STACKSIZE_BYTES);

    typedef         uint8_t     Payload;

    typedef enum SDC_ERRORCode {
        SDC_OK                   = 0,
        SDC_NULL_PARAMETER_ERROR = -1,
        SDC_FSYNC_ERROR          = -2,
        SDC_FWRITE_ERROR         = -3,
        SDC_UNKNOWN_ERROR        = -99
    } SDC_ERRORCode;

    /*! Start of a list of codes for events
    */
    typedef enum EVENTCode {
        OK_EVENT = 0,
        PWR_RESET,
        WDG_RESET,
        OTH_RESET,
        ADIS_INT,
        MPL_INT,
        MPU_INT,
        FCF_MSG,
        FCF_CMD
    } EVENTCode;

    /*! Start of a list of codes for messages
    */
    typedef enum EVENTCode {
        GENERIC_MESSAGE = 0,
        EVENT_MESSAGE,
        ADIS_MESSAGE,
        MPL_MESSAGE,
        MPU_MESSAGE,
        OTHER_MESSAGE
    } EVENTCode;

    struct Message_head {
        char                 ID[4];
        uint8_t              psas_time_ns[6];
        uint16_t             data_length;
    } __attribute__((packed));
    typedef struct Message_head Message_head;

    struct GENERIC_message {
        Message_head         mh;
        Payload              data[SDC_MAX_PAYLOAD_BYTES];
    } __attribute__((packed));
    typedef struct GENERIC_message GENERIC_message;

    void            InsertHandler(eventid_t id) ;
    void            RemoveHandler(eventid_t id) ;
    void            sdc_tmr_init(void *p) ;

    void            sdc_read_fp_index();
    void            sdc_write_fp_index();
    void            sdc_set_fp_index();
    void            sdc_reset_fp_index();

    SDC_ERRORCode   sdc_write_log_message(FIL* DATAFil, GENERIC_message* d, unsigned int* bw) ;
    FRESULT         sdc_scan_files(BaseSequentialStream *chp, char *path) ;
    msg_t           sdlog_thread(void *p) ;

#ifdef __cplusplus
}
#endif

//! @}

#endif

