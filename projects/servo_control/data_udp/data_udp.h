
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
 * @addtogroup dataudp
 * @{
 */

#ifndef _DATA_UDP_H
#define _DATA_UDP_H

#include "lwip/ip_addr.h"
#include "psas_packet.h"

#define DATA_UDP_SEND_THREAD_STACK_SIZE      512
#define DATA_UDP_RECEIVE_THREAD_STACK_SIZE   512

#define DATA_UDP_RX_THD_PRIORITY             (LOWPRIO)
#define DATA_UDP_THREAD_PRIORITY             (LOWPRIO + 2)

#define PSAS_IP_BROADCAST                    ((uint32_t)0xffffffffUL)

#define DATA_UDP_MSG_SIZE                    50


extern WORKING_AREA(wa_data_udp_send_thread,    DATA_UDP_SEND_THREAD_STACK_SIZE);
extern WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_RECEIVE_THREAD_STACK_SIZE);

#ifdef __cplusplus
extern "C" {
#endif
  msg_t data_udp_tx_launch_det(void *p) ;
  msg_t data_udp_receive_thread(void *p);

#ifdef __cplusplus
}
#endif

#endif

//! @}

