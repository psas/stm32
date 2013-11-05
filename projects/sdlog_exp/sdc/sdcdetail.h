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
extern          WORKING_AREA(wa_sdlog_thread, SDC_THREAD_STACKSIZE_BYTES);

#define         SDC_THREAD_STACKSIZE_BYTES                  2048

/* A 166 byte message written at 1000hz will use 4GB in about 6.5 Hours */

#define         SDC_MAX_PAYLOAD_BYTES                       150

   msg_t           sdlog_thread(void *p) ;

#ifdef __cplusplus
}
#endif
//! @}

#endif

