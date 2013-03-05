/*! \file extdetail.c
 *
 */

/*!
 * \defgroup threaddetail Thread details
 * @{
 */

#include "threaddetail.h"


//Thread            *tp_dio1         = NULL;
//Thread            *tp_spi_cb       = NULL;
//Thread            *tp_spi_newdata  = NULL;

EventSource        wkup_event;
EventSource        dio1_event;
EventSource        spi_cb_txdone_event;
EventSource        spi_cb_newdata;
EventSource        spi_cb_releasebus;



//! @}
