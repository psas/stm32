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

/*! \brief MPU Sensor datapath node.
 * \defgroup dataudp-mpu MPU Data Sensor UDP PSAS Experiment
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/ip_addr.h"

#include "usbdetail.h"
#include "fc_net.h"
#include "device_net.h"

#include "ADIS16405.h"
#include "MPU9150.h"
#include "MPL3115A2.h"
#include "psas_packet.h"

#include "data_udp.h"

#define LWIP_NETCONN 1
#if LWIP_NETCONN

EventSource                             fc_req_reset_event;

static        MPU9150_MAC_info          mpu9150_mac_info;
static        ADIS16405_MAC_info        adis16405_mac_info;
static        MPL3115A2_MAC_info        mpl3115a2_mac_info;


static void log_error(volatile char *s) {
#if DEBUG_SENSOR_UDP
	static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
	chprintf(chp, "E:%s\r\n", s);
#else
	(void) s;
#endif
}

/*! \brief Initialize events for threads
 *
 */
void data_udp_init(void) {
	chEvtInit(&fc_req_reset_event);
}

/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void data_udp_send_mpl3115a2_data(eventid_t id) {
    (void) id;
    uint8_t*                  data;
    MPL_packet                packet;
    const char                myid[(sizeof("MPL3")-1)] = "MPL3";

    memset (&packet.timestamp, 0, sizeof(packet.timestamp));

    strncpy(packet.ID, myid, sizeof(myid));

    memcpy(&packet.data, (void*) &mpl3115a2_current_read, sizeof(MPL3115A2_read_data) );

    packet.data_length         =  (uint16_t) sizeof(MPL3115A2_read_data);
    mpl3115a2_mac_info.buf     =  netbuf_new();

    data    =  netbuf_alloc(mpl3115a2_mac_info.buf, sizeof(packet));
    if(data != NULL) {
        memcpy (data, (void*) &packet, sizeof(packet));
        netconn_send(mpl3115a2_mac_info.conn, mpl3115a2_mac_info.buf);
        netbuf_delete(mpl3115a2_mac_info.buf);
    }
}
/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void data_udp_send_mpu9150_data(eventid_t id) {
	(void) id;
	uint8_t*                  data;
	MPU_packet                packet;
	const char                myid[(sizeof("MPU9")-1)] = "MPU9";

	memset (&packet.timestamp, 0, sizeof(packet.timestamp));

	strncpy(packet.ID, myid, sizeof(myid));

	memcpy(&packet.data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data) );

	packet.data_length       =  (uint16_t) sizeof(MPU9150_read_data);
	mpu9150_mac_info.buf     =  netbuf_new();

	data    =  netbuf_alloc(mpu9150_mac_info.buf, sizeof(packet));
	if(data != NULL) {
		memcpy (data, (void*) &packet, sizeof(packet));

		//palSetPad(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
		netconn_send(mpu9150_mac_info.conn, mpu9150_mac_info.buf);
		//palClearPad(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
		netbuf_delete(mpu9150_mac_info.buf);
	}
}

/*! \brief event handler for adis16405 udp data
 *  send one packet of adis16405 data on event.
 */
static void data_udp_send_adis16405_data(eventid_t id) {
	(void) id;
	ADIS_packet               packet;
    const char                myid[(sizeof("ADIS")-1)] = "ADIS";

	uint8_t*                  data;

	memset (&packet.timestamp, 0, sizeof(packet.timestamp));

	strncpy(packet.ID, myid, sizeof(myid));

	memcpy(&packet.data, (void*) &adis16405_burst_data, sizeof(ADIS16405_burst_data) );

	packet.data_length       =  (uint16_t) sizeof(ADIS16405_burst_data);

	adis16405_mac_info.buf     =  netbuf_new();

	data    =  netbuf_alloc(adis16405_mac_info.buf, sizeof(ADIS_packet));
	if(data != NULL) {
		memcpy (data, (void*) &packet, sizeof(ADIS_packet));
		netconn_send(adis16405_mac_info.conn, adis16405_mac_info.buf);
		netbuf_delete(adis16405_mac_info.buf);
	}
}

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);
msg_t data_udp_send_thread(void *p) {
	void * arg __attribute__ ((unused)) = p;

	static const evhandler_t evhndl_imu_a[]       = {
			data_udp_send_mpu9150_data,
			data_udp_send_mpl3115a2_data,
			data_udp_send_adis16405_data
	};
	struct EventListener     evl_mpu9150;
    struct EventListener     evl_mpl3115a2;
	struct EventListener     evl_adis16405;

	err_t                    err_mpu_conn;
	err_t                    err_mpl_conn;
	err_t                    err_adis_conn;

	ip_addr_t                ip_addr_sensor;
	ip_addr_t                ip_addr_fc;

	chRegSetThreadName("data_udp_send_thread");

	chEvtRegister(&mpu9150_data_event,                   &evl_mpu9150,           0);
	chEvtRegister(&mpl3115a2_data_event,                 &evl_mpl3115a2,         1);
	chEvtRegister(&adis_spi_burst_data_captured,         &evl_adis16405,         2);

	IMU_A_IP_ADDR(&ip_addr_sensor);
	IP_PSAS_FC(&ip_addr_fc);

	mpu9150_mac_info.conn   = netconn_new( NETCONN_UDP );
	if(mpu9150_mac_info.conn == NULL) {
		log_error("mpu new conn is null");
		while(1);
	}

    mpl3115a2_mac_info.conn   = netconn_new( NETCONN_UDP );
    if(mpl3115a2_mac_info.conn == NULL) {
        log_error("mpl new conn is null");
        while(1);
    }

	adis16405_mac_info.conn   = netconn_new( NETCONN_UDP );
	if(adis16405_mac_info.conn == NULL) {
		log_error("adis new conn is null");
		while(1);
	}

	/* Bind to the local address, or to ANY address */
	//	netconn_bind(conn, NULL, DATA_UDP_TX_THREAD_PORT ); //local port, NULL is bind to ALL ADDRESSES! (IP_ADDR_ANY)
	err_mpu_conn   = netconn_bind(mpu9150_mac_info.conn,   &ip_addr_sensor, IMU_A_TX_PORT_MPU ); //local port

	if (err_mpu_conn != ERR_OK) {
		log_error("mpu bind is not OK");
		while(1);
	}

	err_mpl_conn   = netconn_bind(mpl3115a2_mac_info.conn,   &ip_addr_sensor, IMU_A_TX_PORT_MPL ); //local port

	if (err_mpl_conn != ERR_OK) {
	    log_error("mpl bind is not OK");
	    while(1);
	}

	err_adis_conn   = netconn_bind(adis16405_mac_info.conn,   &ip_addr_sensor, IMU_A_TX_PORT_ADIS ); //local port
	if (err_adis_conn != ERR_OK) {
		log_error("adis bind is not OK");
		while(1);
	}

	if ((err_mpu_conn == ERR_OK) && (err_adis_conn == ERR_OK)) {
		/* Connect to specific address or a broadcast address */
		/*
		 * \todo Understand why a UDP needs a connect...
		 *   This may be a LwIP thing that chooses between tcp_/udp_/raw_ connections internally.
		 *
		 */
	    //	netconn_connect(conn, IP_ADDR_BROADCAST, DATA_UDP_TX_THREAD_PORT );
	    err_mpu_conn  = netconn_connect(mpu9150_mac_info.conn,   &ip_addr_fc, FC_LISTEN_PORT_IMU_A );
	    if (err_mpu_conn != ERR_OK) {
	        log_error("mpu port connect is not OK");
	        while(1);
	    }

	    err_mpl_conn  = netconn_connect(mpl3115a2_mac_info.conn,   &ip_addr_fc, FC_LISTEN_PORT_IMU_A );
	    if (err_mpl_conn != ERR_OK) {
	        log_error("mpl port connect is not OK");
	        while(1);
	    }

	    err_adis_conn = netconn_connect(adis16405_mac_info.conn, &ip_addr_fc, FC_LISTEN_PORT_IMU_A);
	    if (err_adis_conn != ERR_OK) {
	        log_error("adis port connect is not OK");
	        while(1);
	    }

	    if(err_mpu_conn == ERR_OK) {
	        while (TRUE) {
	            chEvtDispatch(evhndl_imu_a, chEvtWaitOneTimeout(EVENT_MASK(2)| EVENT_MASK(1)|EVENT_MASK(0), MS2ST(50)));
	        }
	    } else {
	        log_error("Conn not ok");
	    }
	    return RDY_RESET;
	} else {
	    log_error("2 conn not ok");
	}
	return RDY_RESET;
}

static void data_udp_process_rx(char* rxbuf, uint16_t rxbuflen) {
	int    retval = -1;
	const  char     reset_string_cmd[] = "USER_RESET";

	retval   = strncmp( rxbuf, reset_string_cmd, rxbuflen-1);
	if(retval == 0) {
		chEvtBroadcast(&fc_req_reset_event);
	}
}

static void data_udp_rx_serve(struct netconn *conn) {
#if DEBUG_SENSOR_UDP
	static uint8_t       count  = 0;
	static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
#endif

	struct netbuf        *inbuf;

	char                 *buf;

	uint16_t             buflen = 0;

	err_t                err;

	/*
	 * Read the data from the port, blocking if nothing yet there.
	 * We assume the request (the part we care about) is in one netbuf
	 */
	err = netconn_recv(conn, &inbuf);
	if (err == ERR_OK) {
		netbuf_data(inbuf, (void **)&buf, &buflen);
		//palClearPad(TIMEINPUT_PORT, TIMEINPUT_PIN);     // negative pulse for input.
#if DEBUG_SENSOR_UDP
		chprintf(chp, "\r\nsensor rx (from FC): %d ", count++);
#endif
		//palSetPad(TIMEINPUT_PORT, TIMEINPUT_PIN);
#if DEBUG_SENSOR_UDP
		uint16_t             i      = 0;
		for(i=0; i<buflen; ++i) {
			chprintf(chp, "%c", buf[i]);
		}
		chprintf(chp, "\r\n");
#endif
		data_udp_process_rx(buf, buflen);
	}
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
	 * so we have to make sure to deallocate the buffer)
	 */
	netbuf_delete(inbuf);
}

WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_RECEIVE_THREAD_STACK_SIZE);
msg_t data_udp_receive_thread(void *p) {
	void * arg __attribute__ ((unused)) = p;

	err_t             err;

	struct netconn    *conn;

	ip_addr_t         ip_addr_sensor;

	chRegSetThreadName("data_udp_receive_thread");

	IMU_A_IP_ADDR(&ip_addr_sensor);

	/*
	 *  Create a new UDP connection handle
	 */
	conn = netconn_new(NETCONN_UDP);
	LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);

	/*
	 * Bind sensor address to a udp port
	 */
	err = netconn_bind(conn, &ip_addr_sensor, IMU_A_LISTEN_PORT);

	if (err == ERR_OK) {
		while(1) {
			data_udp_rx_serve(conn);
		}
		return RDY_OK;
	} else {
		return RDY_RESET;
	}
}

#endif

//! @}

