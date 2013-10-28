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


extern     bool        fs_ready;
extern     FATFS       SDC_FS;
extern     EventSource sdc_inserted_event;
extern     EventSource sdc_removed_event;

void       sdc_init(void *p);
void       InsertHandler(eventid_t id);
void       RemoveHandler(eventid_t id);


#ifdef __cplusplus
}
#endif

//! @}

#endif
