
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

/** @brief IP Address. */
#if !defined(LWIP_IPADDR) || defined(__DOXYGEN__)
#define LWIP_IPADDR(p)                      IP4_ADDR(p, 192, 168, 1, 20)
#endif

/** @brief IP Gateway. */
#if !defined(LWIP_GATEWAY) || defined(__DOXYGEN__)
#define LWIP_GATEWAY(p)                     IP4_ADDR(p, 192, 168, 1, 1)
#endif

/** @brief IP netmask. */
#if !defined(LWIP_NETMASK) || defined(__DOXYGEN__)
#define LWIP_NETMASK(p)                     IP4_ADDR(p, 255, 255, 255, 0)
#endif

#ifndef DATA_UDP_THREAD_STACK_SIZE
#define DATA_UDP_THREAD_STACK_SIZE   1024
#endif

#ifndef DATA_UDP_THREAD_PORT
#define DATA_UDP_THREAD_PORT         35000
#endif

#ifndef DATA_UDP_THREAD_PRIORITY
#define DATA_UDP_THREAD_PRIORITY     (LOWPRIO + 2)
#endif


#define PSAS_IP_BROADCAST            0xffffffff

	// htonl = Host TO Network Long, converts a long from host byte order to network byte order
	// ntohl = Network TO Host Long, you get the idea.
	// 192.168.0.91 is 0xC0A8005B
	// 192.168.1.30	is 0xC0A8011E
#define PSAS_IP_HOST                 (htonl(0xC0A8011E))


extern WORKING_AREA(wa_data_udp_server, DATA_UDP_THREAD_STACK_SIZE);

#ifdef __cplusplus
extern "C" {
#endif
  msg_t data_udp_server(void *p);
#ifdef __cplusplus
}
#endif

#endif

//! @}

