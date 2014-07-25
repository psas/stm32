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

#include "MPL3115A2.h"

static int sendsocket;

static const struct swap burst_swaps[] = {
	SWAP_FIELD(struct MPL3115A2Data, status),
	SWAP_FIELD(struct MPL3115A2Data, pressure),
	SWAP_FIELD(struct MPL3115A2Data, temperature),
	{0},
};

static void mpl_handler(eventid_t id UNUSED){

	struct MPL3115A2Data data;
	uint8_t buffer[7];

	palTogglePad(GPIOF, GPIOF_PIN14);
	MPL3115A2GetData(&data);

	write_swapped(burst_swaps, &data, buffer);
	if(write(sendsocket, buffer, sizeof(buffer)) < 0){
		ledError();
	}

}


void main(void){
	halInit();
	chSysInit();

	ledStart(NULL);

	/* Start lwip */
	lwipThreadStart(SENSOR_LWIP);

	/* Create the MPL out socket, connecting as it only sends to one place */
	sendsocket = get_udp_socket(MPL_ADDR);
	if(sendsocket < 0){
		ledError();
	} else if(connect(sendsocket, FC_ADDR, sizeof(struct sockaddr)) < 0){
		ledError();
	}

	static struct MPL3115A2Config conf = {
		.i2cd = &I2CD2,
		.pins = {.SDA = {GPIOF, GPIOF_PIN0}, .SCL = {GPIOF, GPIOF_PIN1}},
		.interrupt = {GPIOF, GPIOF_PIN3}
	};
	MPL3115A2Start(&conf);

	/* Manage MPL events */
	struct EventListener drdy;
	static const evhandler_t evhndl[] = {
		mpl_handler
	};
	chEvtRegister(&MPL3115A2DataEvt, &drdy, 0);
	while(TRUE){
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
