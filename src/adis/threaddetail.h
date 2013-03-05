/*! \file extdetail.h
 *
 */

#ifndef _THREADDETAIL_H
#define _THREADDETAIL_H

/*!
 * \addtogroup threaddetail
 * @{
 */
#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

enum thread_messages {
	ADIS_EMPTY_MSG = 0,
	ADIS_ERROR_MSG,
	ADIS_DIO1_MSG,
	ADIS_CB_MSG,
	ADIS_NEWDATA_MSG
};

extern           Thread          *tp_dio1;
extern           Thread          *tp_spi_cb;
extern           Thread          *tp_spi_newdata;

extern           EventSource     wkup_event;
extern           EventSource     dio1_event;
extern           EventSource     spi_cb_event;
extern           EventSource     spi_cb_event2;
extern           EventSource     spi_newdata_event;


//! @}


#endif
