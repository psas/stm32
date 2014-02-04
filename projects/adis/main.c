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

#include "utils_sockets.h"
#include "utils_led.h"
#include "net_addrs.h"

#include "ADIS16405.h"

#define GPIOF_LED_RED GPIOF_PIN12
#define GPIOF_LED_GREEN GPIOF_PIN13
#define GPIOF_LED_BLUE GPIOF_PIN14

static int sendsocket;

void serialize_adis(ADIS16405_burst_data * data, uint16_t * buffer){
    buffer[0] = htons(data->supply_out);
    buffer[1] = htons(data->xgyro_out);
    buffer[2] = htons(data->ygyro_out);
    buffer[3] = htons(data->zgyro_out);
    buffer[4] = htons(data->xaccl_out);
    buffer[5] = htons(data->yaccl_out);
    buffer[6] = htons(data->zaccl_out);
    buffer[7] = htons(data->xmagn_out);
    buffer[8] = htons(data->ymagn_out);
    buffer[9] = htons(data->zmagn_out);
    buffer[10] = htons(data->temp_out);
    buffer[11] = htons(data->aux_adc);
}

static void adis_drdy_handler(eventid_t id __attribute__((unused))){
    ADIS16405_burst_data data;
    uint16_t buffer[12]; //FIXME: buffer size
    palSetPad(GPIOF, GPIOF_LED_BLUE);

    adis_get_data(&data);
    serialize_adis(&data, buffer);
    if(write(sendsocket, buffer, sizeof(buffer)) < 0){
        palSetPad(GPIOF, GPIOF_LED_RED);
    }

    palClearPad(GPIOF, GPIOF_LED_BLUE);
}

void main(void){
	halInit();
	chSysInit();

	static struct led_config led_cfg = {
	        .cycle_ms = 500,
	        .start_ms = 0,
	        .led = {
	                {GPIOC, GPIOC_LED},
	                {GPIOF, GPIOF_LED_RED},
	                {GPIOF, GPIOF_LED_GREEN},
	                {GPIOF, GPIOF_LED_BLUE},
	                {0, 0}
	        }
	};
	led_init(&led_cfg);

    /* Start lwip */
    chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, SENSOR_LWIP);

    /* Create the ADIS out socket, connecting as it only sends to one place */
    sendsocket = get_udp_socket(ADIS_ADDR);
    if(sendsocket < 0){
        palSetPad(GPIOF, GPIOF_LED_RED);
    } else if(connect(sendsocket, FC_ADDR, sizeof(struct sockaddr)) < 0){
        palSetPad(GPIOF, GPIOF_LED_RED);
    }

    adis_init(&adis_olimex_e407);

    /* Manage ADIS events */
    struct EventListener drdy;
    static const evhandler_t evhndl[] = {
            adis_drdy_handler
    };
    chEvtRegister(&adis_data_ready, &drdy, 0);
	while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
