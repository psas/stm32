
/*!
 * \file data_udp.h
 */

/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

 /* \brief UDP Server wrapper thread
 * @addtogroup dataudp-mpu
 * @{
 */

#ifndef _DATA_UDP_H
#define _DATA_UDP_H

#include "lwip/ip_addr.h"


#define DATA_UDP_SEND_THREAD_STACK_SIZE      512
#define DATA_UDP_RECEIVE_THREAD_STACK_SIZE   512

#define DATA_UDP_RX_THD_PRIORITY             (LOWPRIO)
#define DATA_UDP_THREAD_PRIORITY             (LOWPRIO + 2)

#define PSAS_IP_BROADCAST                    ((uint32_t)0xffffffffUL)

#define DATA_UDP_MSG_SIZE                    128

/*
 * Pin alias names for testing
 */
#define TIMEOUTPUT_PORT             GPIOD
#define TIMEINPUT_PORT              GPIOD
#define TIMEOUTPUT_PIN              GPIOD_PIN5
#define TIMEINPUT_PIN               GPIOD_PIN3


/*! \typedef mpu9150 MAC Connection info
 *
 */
typedef struct mpu9150_MAC_info {
	struct     netconn    *conn ;
	struct     netbuf     *buf ;
	char                   msg[DATA_UDP_MSG_SIZE] ;
} MPU9150_MAC_info;

extern WORKING_AREA(wa_data_udp_send_thread,    DATA_UDP_SEND_THREAD_STACK_SIZE);
extern WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_RECEIVE_THREAD_STACK_SIZE);

extern EventSource                           mpu9150_data_event;

#ifdef __cplusplus
extern "C" {
#endif
  void data_udp_init(void) ;
  msg_t data_udp_send_thread(void *p);
  msg_t data_udp_receive_thread(void *p);


#ifdef __cplusplus
}
#endif

#endif

//! @}

