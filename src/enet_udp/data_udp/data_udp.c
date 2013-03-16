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

#include <string.h>
#include "ch.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "data_udp.h"

#define LWIP_NETCONN 1
#if LWIP_NETCONN


WORKING_AREA(wa_data_udp_server, DATA_UDP_THREAD_STACK_SIZE);

msg_t data_udp_server(void *p) {
	void * arg __attribute__ ((unused)) = p;

	struct     netconn    *conn;
	char                   msg[]="PSAS Rockets!" ;
	struct     netbuf     *buf;
	char*                  data;


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
		memcpy (data, msg, sizeof (msg));
		netconn_send(conn, buf);
		netbuf_delete(buf); // De-allocate packet buffer
	    chThdSleepMilliseconds(500);
	}
}


#endif

//! @}

