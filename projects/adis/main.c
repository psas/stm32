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

#include "usbdetail.h"

#include "ADIS16405.h"


void cmd_pid(BaseSequentialStream *chp, int argc __attribute__((unused)), char *argv[]) {
    (void)argv;
    chprintf(chp, "PID: %x\n\r", adis_get(ADIS_PRODUCT_ID));
}

static void adis_drdy_handler(eventid_t id __attribute__((unused)) ){
    ADIS16405_burst_data data;
    adis_get_burst(&data);
    chprintf(getActiveUsbSerialStream(), "Accel x:%d y:%d z:%d\r\n", data.xaccl_out, data.yaccl_out, data.zaccl_out);
}

void main(void) {

	halInit();
	chSysInit();

	adis_init(&adis_olimex_e407);

    static const ShellCommand commands[] = {
            {"pid", cmd_pid},
            {NULL, NULL}
    };
    usbSerialShellStart(commands);

    struct EventListener drdy;
    chEvtRegister(&adis_data_ready, &drdy, 0);

    static const evhandler_t evhndl[]       = {
            adis_drdy_handler
    };

	while(TRUE){
	    chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
