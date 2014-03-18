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

#define UNUSED __attribute__((unused))

static int adis_socket, mpu_socket, mpl_socket;

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


/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void send_mpl3115a2_data(eventid_t id UNUSED) {
    uint8_t data[sizeof(MPL_packet)];

    memcpy(&packet.data, (void*) &mpl3115a2_current_read, sizeof(MPL3115A2_read_data) );

    memcpy (data, (void*) &packet, sizeof(packet));
    write(mpl_socket, data, sizeof(data));
}
/*! \brief event handler for mpu9150 udp data
 *  send one packet of mpu9150 data on event.
 */
static void send_mpu9150_data(eventid_t id UNUSED) {
    uint8_t data[sizeof(MPU_packet)];

    memcpy(&packet.data, (void*) &mpu9150_current_read, sizeof(MPU9150_read_data) );

    memcpy (data, (void*) &packet, sizeof(packet));
    write(mpu_socket, data, sizeof(data));
}

/*! \brief event handler for adis16405 udp data
 *  send one packet of adis16405 data on event.
 */
static void send_adis16405_data(eventid_t id UNUSED) {

    uint8_t data[sizeof(ADIS_packet)];

    ADIS16405_burst_data burst;
    adis_get_data(&burst);

    memcpy (data, (void*) &packet, sizeof(packet));
    write(adis_socket, data, sizeof(data));
}


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

    /* Set up events */
    static const evhandler_t evhndl_main[]  = {
        sdc_insert_handler,
        sdc_remove_handler,
        send_mpu9150_data,
        send_mpl3115a2_data,
        send_adis16405_data
    };
    struct EventListener     el0, el1;
    struct EventListener     evl_mpu9150;
    struct EventListener     evl_mpl3115a2;
    struct EventListener     evl_adis16405;
    chEvtRegister(&sdc_inserted_event,   &el0, 0);
    chEvtRegister(&sdc_removed_event,    &el1, 1);
    chEvtRegister(&mpu9150_data_event, &evl_mpu9150, 2);
    chEvtRegister(&mpl3115a2_data_event, &evl_mpl3115a2, 3);
    chEvtRegister(&ADIS16405_data_ready, &evl_adis16405, 4);

    usbSerialShellStart(commands);
    while (TRUE) {
       chEvtDispatch(evhndl_main, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(50)));
    }
}

