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

typedef struct logdata {
   uint32_t     index;
   RTCTime      *timespec;
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

