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
#include "data_udp.h"
#include "pwm_config.h"
#include "psas_packet.h"
#include "math.h"

#define LWIP_NETCONN 1
#if LWIP_NETCONN

// WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

enum {
	PWM_ENABLE = 0,
	PWM_DISABLE
};

msg_t data_udp_tx_launch_det(void *p) {
    bool*                launchdet ;
    launchdet = (bool*) p;

    err_t                 err;

    struct     netconn    *conn;
    struct     netbuf     *buf;

    char*                  data;

    RC_LAUNCH_DETECT_STRUCT_TYPE  msg;

    ip_addr_t              ip_addr_rc;
    ip_addr_t              ip_addr_fc;

    UNICAST_IP_ADDR(&ip_addr_rc);
    IP_PSAS_FC(&ip_addr_fc);

    conn   = netconn_new( NETCONN_UDP );
    LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);


    memset(&msg, 0, sizeof(RC_LAUNCH_DETECT_STRUCT_TYPE));

    msg.launch_detect = (*launchdet ? 1 : 0);

    /* Bind to the local address, or to ANY address */
    //	netconn_bind(conn, NULL, DATA_UDP_TX_THREAD_PORT ); //local port, NULL is bind to ALL ADDRESSES! (IP_ADDR_ANY)
    err    = netconn_bind(conn, &ip_addr_rc, UNICAST_TX_PORT ); //local port
    if (err == ERR_OK) {
        /* Connect to specific address or a broadcast address */
        /*
         * \todo Understand why a UDP needs a connect...
         *   This may be a LwIP thing that chooses between tcp_/udp_/raw_ connections internally.
         *
         */
        //	netconn_connect(conn, IP_ADDR_BROADCAST, DATA_UDP_TX_THREAD_PORT );
        err = netconn_connect(conn, &ip_addr_fc, FC_LISTEN_PORT );
        if(err == ERR_OK) {
            buf     =  netbuf_new();
            data    =  netbuf_alloc(buf, sizeof(RC_LAUNCH_DETECT_STRUCT_TYPE));
            msg.launch_detect = (*launchdet ? 1 : 0);
            memcpy (data, &msg, sizeof(RC_LAUNCH_DETECT_STRUCT_TYPE));
            //palSetPad(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
            netconn_send(conn, buf);
            //palClearPad(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
            netbuf_delete(buf); // De-allocate packet buffer
           // chThdSleepMilliseconds(500);
            netconn_delete(conn);
            return RDY_OK;
        } else {
            netconn_delete(conn);
            return RDY_RESET;
        }
    } else {
        netconn_delete(conn);
        return RDY_RESET;
    }
    netconn_delete(conn);
    return RDY_OK;
}

static void data_udp_rx_serve(struct netconn *conn) {
	//BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;
	RC_OUTPUT_STRUCT_TYPE  rc_packet;

	//static uint8_t       count  = 0;
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;

	err_t                err;

	/*
	 * Read the data from the port, blocking if nothing yet there.
	 * We assume the request (the part we care about) is in one netbuf
	 */
	err = netconn_recv(conn, &inbuf);
	if (err == ERR_OK) {
		netbuf_data(inbuf, (void **)&buf, &buflen);
		memcpy (&rc_packet, buf, sizeof (RC_OUTPUT_STRUCT_TYPE));
		if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
		    uint16_t width = rc_packet.u16ServoPulseWidthBin14;
		    double   ms_d  = width/pow(2,14);

		    //chprintf(chp, "\r\n%u\r\n", (uint32_t) (ms_d * 1000));
		    double   us_d  = ms_d * 1000;
            //chprintf(chp, "%u\r\n\r\n", (uint32_t) (us_d * 1000));

		    pwm_set_pulse_width_ticks(pwm_us_to_ticks(us_d));
		} else {
			pwmDisableChannel(&PWMD4, 3);
		}
	}
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
	 * so we have to make sure to deallocate the buffer)
	 */
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

	err_t             err;

	struct netconn    *conn;

	ip_addr_t         ip_addr_rc;

	chRegSetThreadName("data_udp_receive_thread");

	UNICAST_IP_ADDR(&ip_addr_rc);

	/*
	 *  Create a new UDP connection handle
	 */
	conn = netconn_new(NETCONN_UDP);
	LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);

	/*
	 * Bind sensor address to a udp port
	 */
	err = netconn_bind(conn, &ip_addr_rc, UNICAST_LISTEN_PORT);

	if (err == ERR_OK) {
		while(1) {
			data_udp_rx_serve(conn);
		}
		return RDY_OK;
	} else {
		return RDY_RESET;
	}
}

msg_t data_udp_multicast_receive_thread(void *p) {
	void * arg __attribute__ ((unused)) = p;
	err_t err;

}

#endif

//! @}

