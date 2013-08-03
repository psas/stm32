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

extern          bool        fs_ready;
extern          FATFS       SDC_FS;

extern          EventSource inserted_event;
extern          EventSource removed_event;

void            InsertHandler(eventid_t id) ;
void            RemoveHandler(eventid_t id) ;
void            sdc_tmr_init(void *p) ;

FRESULT         sdc_scan_files(BaseSequentialStream *chp, char *path) ;

#ifdef __cplusplus
}
#endif



//! @}


#endif

