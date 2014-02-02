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

#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "usbdetail.h"
#include "net_addrs.h"
#include "utils_sockets.h"

#include "ADIS16405.h"
#include "MPU9150.h"
#include "MPL3115A2.h"
#include "psas_packet.h"

#include "data_udp.h"

#define UNUSED __attribute__((unused))

int adis_socket, mpu_socket, mpl_socket;

static void log_error(volatile char *s) {
#if DEBUG_SENSOR_UDP
	BaseSequentialStream *chp = getUsbStream();
	chprintf(chp, "E:%s\r\n", s);
#else
	(void) s;
#endif
}
/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void data_udp_send_mpl3115a2_data(eventid_t id UNUSED) {
    uint8_t data[sizeof(MPL_packet)];
    MPL_packet                packet;
    const char                myid[(sizeof("MPL3")-1)] = "MPL3";

    memset (&packet.timestamp, 0, sizeof(packet.timestamp));
    strncpy(packet.ID, myid, sizeof(myid));
    memcpy(&packet.data, (void*) &mpl3115a2_current_read, sizeof(MPL3115A2_read_data) );
    packet.data_length =  (uint16_t) sizeof(MPL3115A2_read_data);

    memcpy (data, (void*) &packet, sizeof(packet));
    write(mpl_socket, data, sizeof(data));
}
/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void data_udp_send_mpu9150_data(eventid_t id UNUSED) {
	uint8_t data[sizeof(MPU_packet)];
	MPU_packet                packet;
	const char                myid[(sizeof("MPU9")-1)] = "MPU9";

	memset (&packet.timestamp, 0, sizeof(packet.timestamp));
	strncpy(packet.ID, myid, sizeof(myid));
	memcpy(&packet.data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data) );
	packet.data_length       =  (uint16_t) sizeof(MPU9150_read_data);

    memcpy (data, (void*) &packet, sizeof(packet));
    write(mpu_socket, data, sizeof(data));
}

/*! \brief event handler for adis16405 udp data
 *  send one packet of adis16405 data on event.
 */
static void data_udp_send_adis16405_data(eventid_t id UNUSED) {
	ADIS_packet               packet;
    const char                myid[(sizeof("ADIS")-1)] = "ADIS";
	uint8_t data[sizeof(ADIS_packet)];

	ADIS16405_burst_data burst;
	adis_get_data(&burst);

	memset (&packet.timestamp, 0, sizeof(packet.timestamp));
	strncpy(packet.ID, myid, sizeof(myid));
	memcpy(&packet.data, (void*) &burst, sizeof(ADIS16405_burst_data) );
	packet.data_length       =  (uint16_t) sizeof(ADIS16405_burst_data);

	memcpy (data, (void*) &packet, sizeof(packet));
	write(adis_socket, data, sizeof(data));
}

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);
msg_t data_udp_send_thread(void *p UNUSED) {
    chRegSetThreadName("data_udp_send_thread");

	static const evhandler_t evhndl_imu_a[]       = {
			data_udp_send_mpu9150_data,
			data_udp_send_mpl3115a2_data,
			data_udp_send_adis16405_data
	};
	struct EventListener     evl_mpu9150;
    struct EventListener     evl_mpl3115a2;
	struct EventListener     evl_adis16405;

	chEvtRegister(&mpu9150_data_event,                   &evl_mpu9150,           0);
	chEvtRegister(&mpl3115a2_data_event,                 &evl_mpl3115a2,         1);
	chEvtRegister(&adis_data_ready,         &evl_adis16405,         2);

	adis_socket = get_udp_socket(ADIS_ADDR);
	mpu_socket = get_udp_socket(MPU_ADDR);
	mpl_socket = get_udp_socket(MPL_ADDR);

	if(adis_socket < 0 || mpu_socket < 0 || mpl_socket < 0){
	    log_error("Failed to get a sensor socket");
	    return RDY_RESET;
	}

	connect(adis_socket, FC_ADDR, sizeof(struct sockaddr));
	connect(mpu_socket, FC_ADDR, sizeof(struct sockaddr));
	connect(mpl_socket, FC_ADDR, sizeof(struct sockaddr));

    while (TRUE) {
        chEvtDispatch(evhndl_imu_a, chEvtWaitOneTimeout(EVENT_MASK(2)| EVENT_MASK(1)|EVENT_MASK(0), MS2ST(50)));
    }

	return RDY_RESET;
}

