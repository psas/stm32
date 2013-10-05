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

#define         SDLOG_THREAD_STACKSIZE_BYTES                  1024

extern          bool        fs_ready;
extern          FATFS       SDC_FS;

extern          EventSource inserted_event;
extern          EventSource removed_event;
extern          WORKING_AREA(wa_sdlog_thread, SDLOG_THREAD_STACKSIZE_BYTES);

/*! Start of a list of codes for events
 */
typedef enum EVNTCode {
        OK_EVENT  = 0,
	PWR_RESET, 
	WDG_RESET, 
	ADIS_DATA, 
	OTH_RESET, 
	ADIS_INT, 
	MPL_INT, 
	MPU_INT, 
	FCF_MSG 
} EVNTCode;

struct Message_head {
    char                 ID[4];
    uint8_t              timestamp[6];
    uint16_t             data_length;
} __attribute__((packed));
typedef struct Message_head Message_head;

struct ADIS_message {
        Message_head         message_head;
        ADIS16405_burst_data data;
} __attribute__((packed));
typedef struct ADIS_message ADIS_message;

typedef struct Event_message {
        Message_head         message_head;
        int                  event;
        uint8_t[10]          event_str;   // human readable note
} __attribute__((packed));
typedef struct EVNT_message EVNT_message;

typedef struct MPU_message {
        Message_head         message_head;
        mpudata              dataevent;
}__attribute__((packed));
typedef struct MPU_message MPU_message;

typedef struct MPL_message {
        Message_head         message_head;
        mpldata              data;
}__attribute__((packed));
typedef struct MPL_message MPL_message;

typedef struct logdata {
   uint32_t     index;
   RTCTime      timespec;
} Logdata;

void            InsertHandler(eventid_t id) ;
void            RemoveHandler(eventid_t id) ;
void            sdc_tmr_init(void *p) ;

FRESULT         sdc_scan_files(BaseSequentialStream *chp, char *path) ;
msg_t           sdlog_thread(void *p) ;

#ifdef __cplusplus
}
#endif

//! @}

#endif

