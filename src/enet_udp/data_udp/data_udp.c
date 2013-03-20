/*!
 * \file data_udp.c
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

/*
 * This file is a modified version of the lwIP web server demo. The original
 * author is unknown because the file didn't contain any license information.
 */

/*!
 * \brief Experiment with sending sensor data over UDP connection.
 * \defgroup dataudp Data UDP PSAS Experiment
 * @{
 */

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/ip_addr.h"

#include "usbdetail.h"
#include "data_udp.h"

#define LWIP_NETCONN 1
#if LWIP_NETCONN

ip_addr_t ip_addr_fc;

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_send_thread(void *p) {
	void * arg __attribute__ ((unused)) = p;
	uint8_t               count = 0;
	struct     netconn    *conn;
	char                   msg[DATA_UDP_MSG_SIZE] ;
	struct     netbuf     *buf;
	char*                  data;

	  chRegSetThreadName("data_udp_send_thread");

//    netconn_connect(tftptxcon,&modsrv_addr,ip_data_out.port);    // Open connection
//    tftptxcon->pcb.udp->local_port=69;   // Set local (source) port
//    tftpsrvbuf=netbuf_new();        // Create netbuf
//      .....
//
	conn       = netconn_new( NETCONN_UDP );
	netconn_bind(conn, NULL, 35000 ); //local port

	netconn_connect(conn, IP_ADDR_BROADCAST, DATA_UDP_THREAD_PORT );
	for( ;; ){
		buf     =  netbuf_new();
		data    =  netbuf_alloc(buf, sizeof(msg));
		sprintf(msg, "PSAS Rockets! %d", count++);
		memcpy (data, msg, sizeof (msg));
		netconn_send(conn, buf);
		netbuf_delete(buf); // De-allocate packet buffer
	    chThdSleepMilliseconds(500);
	}
}

static void data_udp_rx_serve(struct netconn *conn) {
  BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU1;

  struct netbuf   *inbuf;
  char            *buf;
  uint16_t        buflen = 0;
  uint16_t        i      = 0;
  err_t           err;

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  chprintf(chp, ".w.\r\n");
  err = netconn_recv(conn, &inbuf);
  chprintf(chp, ".+.\r\n");
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void **)&buf, &buflen);
    for(i=0; i<buflen; ++i) {
    	chprintf(chp, "\r\ndata_udp_rx: %d", buf[i]);
    }
	chprintf(chp, "\r\n");
  }
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

/*!
 * Stack area for the data_udp_receive_thread.
 */
WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

/*!
 * data_udp_rx  thread.
 */
msg_t data_udp_receive_thread(void *p) {
  void * arg __attribute__ ((unused)) = p;

  struct netconn *conn;

  chRegSetThreadName("data_udp_receive_thread");

  chThdSleepSeconds(2);

//  IP4_ADDR(&ip_addr_fc, 192,168,0,91);
  IP4_ADDR(&ip_addr_fc, 192,168,0,196);
  /* Create a new UDP connection handle */
  conn = netconn_new(NETCONN_UDP);
  LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);

  netconn_bind(conn, &ip_addr_fc, DATA_UDP_RX_THREAD_PORT);

  while(1) {
    //netconn_connect(conn,  &ip_addr_fc, DATA_UDP_RX_THREAD_PORT );
    data_udp_rx_serve(conn);
  }
  return RDY_OK;
}


#endif

//! @}

