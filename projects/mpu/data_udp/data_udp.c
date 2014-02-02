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
#include "utils_sockets.h"
#include "net_addrs.h"
#include "data_udp.h"

#include "MPU9150.h"

#define UNUSED __attribute__((unused))

int mpu_socket;

/*! \brief Initialize events for threads
 *
 */
void data_udp_init(void) {
		chEvtInit(&mpu9150_data_event);
}

/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void data_udp_send_mpu9150_data(eventid_t id UNUSED) {
	uint8_t data[sizeof(MPU9150_read_data)];
	BaseSequentialStream *chp = getUsbStream();

	chprintf(chp, "ACCL:  x: %d\ty: %d\tz: %d\r\n", mpu9150_current_read.accel_xyz.x, mpu9150_current_read.accel_xyz.y, mpu9150_current_read.accel_xyz.z);

    memcpy (data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data));
    chprintf(chp, "size: %d\r\n", sizeof(MPU9150_read_data));

    write(mpu_socket, data, sizeof(data));
}

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);
msg_t data_udp_send_thread(void *p UNUSED) {
    chRegSetThreadName("data_udp_send_thread");

	static const evhandler_t evhndl_mpu9150[] = {
			data_udp_send_mpu9150_data
	};
	struct EventListener     evl_mpu9150;
	chEvtRegister(&mpu9150_data_event,           &evl_mpu9150,         0);


	mpu_socket = get_udp_socket(MPU_ADDR);
	connect(mpu_socket, FC_ADDR, sizeof(struct sockaddr));

    while (TRUE) {
        chEvtDispatch(evhndl_mpu9150, chEvtWaitOneTimeout(EVENT_MASK(0), MS2ST(50)));
    }
    return -1;
}
