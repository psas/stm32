/*! \file sensor_mpu.h
 */

#ifndef _SENSOR_MPU_H
#define _SENSOR_MPU_H

#define    MPU9150_INT_THREAD_STACKSIZE    512
#define    MPU9150_RESET_THREAD_STACKSIZE  256


extern WORKING_AREA(waThread_mpu9150_int, MPU9150_INT_THREAD_STACKSIZE);
// WORKING_AREA(waThread_mpu9150_reset_req, MPU9150_RESET_THREAD_STACKSIZE);

msg_t Thread_mpu9150_int(void* arg) ;

// msg_t Thread_mpu9150_reset_req(void* arg) ;




#endif




