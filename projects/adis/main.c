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

static void adis_drdy_handler(eventid_t id __attribute__((unused)) ){
//    BaseSequentialStream *chp = getActiveUsbSerialStream();
    ADIS16405_burst_data data;
    uint16_t buffer[25];
    adis_get_data(&data);
//    chprintf(chp, "Accel x:%d y:%d z:%d\r\n", data.xaccl_out, data.yaccl_out, data.zaccl_out);
    serialize_adis(&data, buffer);
    int ret;
    ret = sendto(sendsocket, buffer, 24, 0, (struct sockaddr*)&adis_in, sizeof(adis_in));
    if( ret < 0){
//        chprintf(chp, "Send socket send failure : %d\r\n", ret);
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

    adis_init(&adis_olimex_e407);
    struct EventListener drdy;
    chEvtRegister(&adis_data_ready, &drdy, 0);

    static const evhandler_t evhndl[] = {
            adis_drdy_handler
    };


    chThdCreateStatic(wa_led , sizeof(wa_led) , NORMALPRIO , led, NULL);
    chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 2, lwip_thread, get_adis_addr());
    sendsocket = -1;
    ADIS_IN_ADDR;
    sendsocket = get_udp_socket(ADIS_OUT_ADDR);
//    connect(sendsocket, ADIS_IN_ADDR, sizeof(struct sockaddr_in));
    const ShellCommand commands[] = {
            {NULL, NULL}
    };

    usbSerialShellStart(commands);

	while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
