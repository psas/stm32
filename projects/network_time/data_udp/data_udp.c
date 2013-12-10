/*!
 * \file data_udp.c

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

/*! \brief Sensor datapath node.
 * \defgroup dataudp Data Sensor UDP PSAS Experiment
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
#include "time_net.h"
#include "time_packet.h"
#include "device_net.h"
#include "data_udp.h"
#include "chrtclib.h"
#include "psas_rtc.h"

#define LWIP_NETCONN 1
#if LWIP_NETCONN

//WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);
//
//msg_t data_udp_send_thread(void *p) {
//	void * arg __attribute__ ((unused)) = p;
//
//	err_t                 err;
//	uint8_t               count = 0;
//
//	struct     netconn    *conn;
//	struct     netbuf     *buf;
//
//	char*                  data;
//	char                   msg[DATA_UDP_MSG_SIZE] ;
//
//	ip_addr_t              ip_addr_sensor;
//	ip_addr_t              ip_addr_fc;
//
//	IMU_A_IP_ADDR(&ip_addr_sensor);
//	IP_PSAS_FC(&ip_addr_fc);
//
//	chRegSetThreadName("data_udp_send_thread");
//
//	conn   = netconn_new( NETCONN_UDP );
//
//	/* Bind to the local address, or to ANY address */
//	//	netconn_bind(conn, NULL, DATA_UDP_TX_THREAD_PORT ); //local port, NULL is bind to ALL ADDRESSES! (IP_ADDR_ANY)
//	err    = netconn_bind(conn, &ip_addr_sensor, IMU_A_TX_PORT ); //local port
//
//	if (err == ERR_OK) {
//		/* Connect to specific address or a broadcast address */
//		/*
//		 * \todo Understand why a UDP needs a connect...
//		 *   This may be a LwIP thing that chooses between tcp_/udp_/raw_ connections internally.
//		 *
//		 */
//		//	netconn_connect(conn, IP_ADDR_BROADCAST, DATA_UDP_TX_THREAD_PORT );
//		err = netconn_connect(conn, &ip_addr_fc, FC_LISTEN_PORT_IMU_A );
//		if(err == ERR_OK) {
//			for( ;; ){
//				buf     =  netbuf_new();
//				data    =  netbuf_alloc(buf, sizeof(msg));
//				sprintf(msg, "sensor tx: %d", count++);
//				memcpy (data, msg, sizeof (msg));
//				palSetPad(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
//				netconn_send(conn, buf);
//				palClearPad(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
//				netbuf_delete(buf); // De-allocate packet buffer
//				chThdSleepMilliseconds(500);
//			}
//			return RDY_OK;
//		} else {
//			return RDY_RESET;
//		}
//	} else {
//		return RDY_RESET;
//	}
//}
//
//static void data_udp_rx_serve(struct netconn *conn) {
//	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;
//
//	static uint8_t       count  = 0;
//
//	struct netbuf        *inbuf;
//
//	char                 *buf;
//
//	uint16_t             buflen = 0;
//	uint16_t             i      = 0;
//
//	err_t                err;
//
//	/*
//	 * Read the data from the port, blocking if nothing yet there.
//	 * We assume the request (the part we care about) is in one netbuf
//	 */
//	err = netconn_recv(conn, &inbuf);
//	if (err == ERR_OK) {
//		netbuf_data(inbuf, (void **)&buf, &buflen);
//		palClearPad(TIMEINPUT_PORT, TIMEINPUT_PIN);     // negative pulse for input.
//		chprintf(chp, "\r\nsensor rx (from FC): %d ", count++);
//		palSetPad(TIMEINPUT_PORT, TIMEINPUT_PIN);
//		for(i=0; i<buflen; ++i) {
//			chprintf(chp, "%c", buf[i]);
//		}
//		chprintf(chp, "\r\n");
//	}
//	netconn_close(conn);
//
//	/* Delete the buffer (netconn_recv gives us ownership,
//	 * so we have to make sure to deallocate the buffer)
//	 */
//	netbuf_delete(inbuf);
//}
//
///*!
// * Stack area for the data_udp_receive_thread.
// */
//WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);
//
///*!
// * data_udp_rx  thread.
// */
//msg_t data_udp_receive_thread(void *p) {
//	void * arg __attribute__ ((unused)) = p;
//
//	err_t             err;
//
//	struct netconn    *conn;
//
//	ip_addr_t         ip_addr_sensor;
//
//	chRegSetThreadName("data_udp_receive_thread");
//
//	IMU_A_IP_ADDR(&ip_addr_sensor);
//
//	/*
//	 *  Create a new UDP connection handle
//	 */
//	conn = netconn_new(NETCONN_UDP);
//	LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);
//
//	/*
//	 * Bind sensor address to a udp port
//	 */
//	err = netconn_bind(conn, &ip_addr_sensor, IMU_A_LISTEN_PORT);
//
//	if (err == ERR_OK) {
//		while(1) {
//			data_udp_rx_serve(conn);
//		}
//		return RDY_OK;
//	} else {
//		return RDY_RESET;
//	}
//}
//
//
WORKING_AREA(wa_data_udp_time_control_send_thread, 512);

msg_t data_udp_time_control_send(void *p) {
	err_t	err;
	struct netconn *con;
	struct netbuf *buf;
	char* data;
	time_packet tpacket_rx, tpacket_tx;
	ip_addr_t ip_addr_timeserver;
	uint32_t t1_sec, t1_msec, t1_prime_sec, t1_prime_msec, t2_sec, t2_msec, t2_prime_sec, t2_prime_msec;
	TIME_SERVER_IP_ADDR(&ip_addr_timeserver);
	con   = netconn_new( NETCONN_UDP );
	err = netconn_connect(con, &ip_addr_timeserver, TIME_CLIENT_TX_PORT);
	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;
	chRegSetThreadName("data_udp_time_control_send_thread");

	if (err == ERR_OK) {
		size_t pktSize = sizeof(time_packet);
		chprintf(chp,"%d", pktSize);
		chprintf(chp,"\r\n");
		buf = netbuf_new();
		data = netbuf_alloc(buf, sizeof(time_packet));
		tpacket_tx.msg_type = 1;
		tpacket_tx.t1_secs = 0;
		tpacket_tx.t1_ms = 0;
		tpacket_tx.t2_secs = 0;
		tpacket_tx.t2_ms = 0;
		memcpy (data, (void*) &tpacket_tx, sizeof(time_packet));
		for (;;) {
			netconn_send(con, buf);
			chThdSleepMilliseconds(1000);
		}
	}
}
WORKING_AREA(wa_data_udp_time_control_receive_thread, 512);
msg_t data_udp_time_control_receive(void *p) {
	void * arg __attribute__ ((unused)) = p;
	err_t             err;
	struct netconn    *conn;
	ip_addr_t         ip_addr_sensor;
	chRegSetThreadName("data_udp_time_control_receive_thread");
	TIME_CLIENT_IP_ADDR(&ip_addr_sensor);
	conn = netconn_new(NETCONN_UDP);	
	err = netconn_bind(conn, &ip_addr_sensor, TIME_CLIENT_LISTEN_PORT);
	if (err == ERR_OK) {
		while(1) {
			data_udp_time_control_server(conn);
		}
		return RDY_OK;
	} else {
		return RDY_RESET;
	}
}

static void data_udp_time_control_server(struct netconn *conn) {
	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;
	static uint8_t       count  = 0;
	struct netbuf        *inbuf;
	char                 *buf;
	uint16_t             buflen = sizeof(time_packet);
	uint16_t             i      = 0;
	err_t                err;
	time_packet				tpacket_rx;
	err = netconn_recv(conn, &inbuf);
	RTCTime current_time;
	if (err == ERR_OK) {
		netbuf_data(inbuf, (void **)&buf, &buflen);
		memcpy(&tpacket_rx, buf, sizeof(time_packet));
		chprintf(chp,"we got a packet!\r\n");
		chprintf(chp,"message type: %d\r\n", tpacket_rx.msg_type);
		chprintf(chp,"seconds: %d\r\n", tpacket_rx.t1_secs);
		chprintf(chp,"milliseconds: %d\r\n", tpacket_rx.t1_ms);
		current_time.tv_time = tpacket_rx.t1_secs;
		current_time.tv_msec = tpacket_rx.t1_ms;
		chprintf(chp,"Attempting to set time\r\n");
		rtcSetTime(&RTCD1, &current_time);
		chprintf(chp,"Attempting to set time did not crash board\r\n");

	} else {
		chprintf(chp,"something terrible has happened\r\n");
	}
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
	 * so we have to make sure to deallocate the buffer)
	 */
	netbuf_delete(inbuf);
}

#endif
//! @}

