/*! \file sensor_mpl.h 
 */

/*!
 * \addtogroup sensor_mpl
 * @{
 */


#ifndef _SENSOR_MPL_H
#define _SENSOR_MPL_H value

#define     MPL_INT_THREAD_STACKSIZE_BYTES  256

#ifdef __cplusplus
extern "C" {
#endif


extern WORKING_AREA(waThread_mpl_int_1, MPL_INT_THREAD_STACKSIZE_BYTES);

void mpl_read_handler(eventid_t id) ;
msg_t Thread_mpl_int_1(void *arg) ;

#ifdef __cplusplus
}
#endif

//! @}


#endif
