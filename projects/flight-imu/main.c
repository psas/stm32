#include <stdint.h>

#include "ch.h"
#include "hal.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "rci.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "net_addrs.h"

#include "ADIS16405.h"
#include "BMP180.h"

static struct SeqSocket adis_socket = DECL_SEQ_SOCKET(sizeof(ADIS16405Data));
static struct SeqSocket bmp_socket = DECL_SEQ_SOCKET(sizeof(struct BMP180Data));

static const struct swap adis_swaps[] = {
	SWAP_FIELD(ADIS16405Data, supply_out),
	SWAP_FIELD(ADIS16405Data, xgyro_out),
	SWAP_FIELD(ADIS16405Data, ygyro_out),
	SWAP_FIELD(ADIS16405Data, zgyro_out),
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

static const struct swap bmp_swaps[] = {
	SWAP_FIELD(struct BMP180Data, pressure),
	SWAP_FIELD(struct BMP180Data, temperature),
	{0},
};

static void adis_drdy_handler(eventid_t id UNUSED){
	ADIS16405Data data;
	adis_get_data(&data);
	write_swapped(adis_swaps, &data, adis_socket.buffer);
	seqWrite(&adis_socket, len_swapped(adis_swaps));
}

static void bmp_drdy_handler(eventid_t id UNUSED){
	struct BMP180Data data;
	BMP180_getSample(&data);
	write_swapped(bmp_swaps, &data, bmp_socket.buffer);
	seqWrite(&bmp_socket, len_swapped(bmp_swaps));
}

void bmpid(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret, void * user UNUSED) {
	uint8_t id = 0xAA;
	int r = BMP180_id(&id);
	if(r) {
		ret->data[0] = 'E';
		chsnprintf(ret->data + 1, 4, "%x", r);
		ret->len = 5;
	} else {
		chsnprintf(ret->data, 2, "%x", id);
		ret->len = 2;
	}
}

void main(void){
	watchdogChibiosStart();
	ledStart(NULL);

	struct RCICommand commands[] = {
		RCI_CMD_VERS,
		{"#BMID", bmpid, NULL},
		{NULL}
	};

	/* Start lwip */
	lwipThreadStart(SENSOR_LWIP);
	RCICreate(commands);

	/* Create the ADIS out socket, connecting as it only sends to one place */
	seqSocket(&adis_socket, ADIS_ADDR);
	chDbgAssert(adis_socket.socket >= 0, "ADIS socket failed", NULL);

	seqSocket(&bmp_socket, BMP_ADDR);
	chDbgAssert(bmp_socket.socket >= 0, "BMP socket failed", NULL);

	connect(adis_socket.socket, FC_ADDR, sizeof(struct sockaddr));
	connect(bmp_socket.socket, FC_ADDR, sizeof(struct sockaddr));

	adis_init(&adis_olimex_e407);
	static struct BMP180Config conf = {
		.i2cd = &I2CD2,
		.pins = {.SDA = {GPIOF, GPIOF_PIN0}, .SCL = {GPIOF, GPIOF_PIN1}},
	};
	BMP180_start(&conf);

	/* Manage data events */
	struct EventListener adis_drdy, bmp_drdy, bmp_pump;
	static const evhandler_t evhndl[] = {
			adis_drdy_handler,
			bmp_drdy_handler,
			BMP180_pump,
	};
	chEvtRegister(&ADIS16405_data_ready, &adis_drdy, 0);
	chEvtRegister(&BMP180DataEvt, &bmp_drdy, 1);
	chEvtRegister(&BMP180Timer.et_es, &bmp_pump, 2);
	while(TRUE){
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
