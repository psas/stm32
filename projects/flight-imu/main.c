/*
 * Flight Application IMU
 * This is specific to the Olimex stm32-e407 board modified for flight with IMU sensors.
 */

#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "lwipthread.h"
#include "ff.h"

#include "ADIS16405.h"
#include "MPU9150.h"
#include "MPL3115A2.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_led.h"

#include "usbdetail.h"

#include "data_udp.h"
#include "psas_rtc.h"
#include "psas_sdclog.h"
#include "sdcdetail.h"

#include "main.h"

static const ShellCommand commands[] = {
    {"tree",        cmd_tree},
    {"sdchalt",     cmd_sdchalt},
    {"date",        cmd_date},
    {"mem",         cmd_mem},
    {"threads",     cmd_threads},
    {NULL,          NULL}
};

/*! \typedef mpu9150_config
 *
 * Configuration for the MPU IMU connections
 */
const MPU9150Config mpuconf = {
    .sda = {GPIOF, 0},
    .scl = {GPIOF, 1},
    .interrupt = {GPIOF, 13},
    .I2CD = &I2CD2
};

int main(void) {
    /* Initialize system */
    halInit();
    chSysInit();
    led_init(&e407_led_cfg); //diagnostics

    /* Start the RTC */
    psas_rtc_lld_init();
    psas_rtc_set_fc_boot_mark(&RTCD1); 


    /* Start SD Card logging */
    static const evhandler_t evhndl_main[]  = {
        sdc_insert_handler,
        sdc_remove_handler
    };
    struct EventListener     el0, el1;
    chEvtRegister(&sdc_inserted_event,   &el0, 0);
    chEvtRegister(&sdc_removed_event,    &el1, 1);
    /* Activates SDC driver 1 using default configuration */
    sdcStart(&SDCD1, NULL);
    /* Activates the card insertion monitor */
    sdc_tmr_init(&SDCD1);
    //chThdCreateStatic(wa_sdlog_thread           , sizeof(wa_sdlog_thread)           , NORMALPRIO    , sdlog_thread           , NULL);


    /* Start RocketNet Communication */
    chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, SENSOR_LWIP);
    /*    chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
     *    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);
     */

    MPU9150_init(&mpuconf);
    adis_init(&adis_olimex_e407);

    usbSerialShellStart(commands);
    // It is possible the card is already inserted. Check now by calling insert handler directly.
    sdc_insert_handler(0);

    while (TRUE) {
       chEvtDispatch(evhndl_main, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(50)));
    }
}

