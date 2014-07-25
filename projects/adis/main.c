/*! \file main.c
 *
 * Development for ADIS IMU on ChibiOS
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */
#include <stdint.h>

#include "ch.h"
#include "hal.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "chprintf.h"
#include "rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "net_addrs.h"

#include "ADIS16405.h"

static int sendsocket;

static const struct swap burst_swaps[] = {
	SWAP_FIELD(ADIS16405Data, supply_out),
	SWAP_FIELD(ADIS16405Data, xgyro_out),
	SWAP_FIELD(ADIS16405Data, ygyro_out),
	SWAP_FIELD(ADIS16405Data, xaccl_out),
	SWAP_FIELD(ADIS16405Data, yaccl_out),
	SWAP_FIELD(ADIS16405Data, zaccl_out),
	SWAP_FIELD(ADIS16405Data, xmagn_out),
	SWAP_FIELD(ADIS16405Data, ymagn_out),
	SWAP_FIELD(ADIS16405Data, zmagn_out),
	SWAP_FIELD(ADIS16405Data, temp_out),
	SWAP_FIELD(ADIS16405Data, aux_adc),
	{0},
};

static void adis_drdy_handler(eventid_t id UNUSED){
	ADIS16405Data data;
	uint8_t buffer[sizeof(data)];

	adis_get_data(&data);

	write_swapped(burst_swaps, &data, buffer);
	if(write(sendsocket, buffer, sizeof(buffer)) < 0){
		ledError();
	}
}

static void self_test(struct RCICmdData *cmd UNUSED, struct RCIRetData * ret, void *user_data UNUSED) {
	uint16_t result = adis_self_test();
	chsnprintf(ret->data, 4, "%x", result);
	ret->len = 4;
}

void main(void){
	halInit();
	chSysInit();

	ledStart(NULL);

	struct RCICommand commands[] = {
		{"#adst",self_test,NULL}, // adis self test
		{NULL}
	};

	/* Start lwip */
	lwipThreadStart(SENSOR_LWIP);
	RCICreate(commands);

	/* Create the ADIS out socket, connecting as it only sends to one place */
	sendsocket = get_udp_socket(ADIS_ADDR);
	if(sendsocket < 0){
		ledError();
	} else if(connect(sendsocket, FC_ADDR, sizeof(struct sockaddr)) < 0){
		ledError();
	}

	adis_init(&adis_olimex_e407);

	/* Manage ADIS events */
	struct EventListener drdy;
	static const evhandler_t evhndl[] = {
		adis_drdy_handler
	};
	chEvtRegister(&ADIS16405_data_ready, &drdy, 0);
	while(TRUE){
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
