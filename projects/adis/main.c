/*! \file main.c
 *
 * Development for ADIS IMU on ChibiOS
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "usbdetail.h"
#include "device_net.h"
#include "fc_net.h"

#include "ADIS16405.h"

static int sendsocket;

static void adis_drdy_handler(eventid_t id __attribute__((unused)) ){
    BaseSequentialStream *chp = getActiveUsbSerialStream();
    ADIS16405_burst_data data;
    adis_get_data(&data);
    chprintf(chp, "Accel x:%d y:%d z:%d\r\n", data.xaccl_out, data.yaccl_out, data.zaccl_out);
    if(send(sendsocket, &data, sizeof(data), 0) < 0){
        chprintf(chp, "Send socket send failure\r\t");
    }
}

WORKING_AREA(wa_led, 1024);
msg_t led(void * arg __attribute__((unused))) {
    while (1) {
        palTogglePad(GPIOC, GPIOC_LED);
        chThdSleepMilliseconds(500);
    }
    return -1;
}

void main(void) {
	halInit();
	chSysInit();

    struct EventListener drdy;
    chEvtRegister(&adis_data_ready, &drdy, 0);

    static const evhandler_t evhndl[] = {
            adis_drdy_handler
    };
	adis_init(&adis_olimex_e407);

    chThdCreateStatic(wa_led , sizeof(wa_led) , NORMALPRIO , led, NULL);
    chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 2, lwip_thread, get_adis_addr());

    sendsocket = get_udp_socket(ADIS_OUT_ADDR);
    connect(sendsocket, ADIS_IN_ADDR, sizeof(struct sockaddr_in));

	while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
