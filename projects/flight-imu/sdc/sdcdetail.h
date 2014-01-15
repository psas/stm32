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

   msg_t           sdlog_thread(void *p) ;

#ifdef __cplusplus
}
#endif
//! @}

#endif

