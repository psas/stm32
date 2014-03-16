/*
 * Flight Application IMU
 * This is specific to the Olimex stm32-e407 board modified for flight with IMU sensors.
 */

#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "lwip/ip_addr.h"
#include "lwipopts.h"
#include "lwipthread.h"
#include "ff.h"

#include "MPU9150.h"
#include "ADIS16405.h"
#include "MPL3115A2.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_led.h"

#include "usbdetail.h"

#include "sensor_mpl.h"
#include "sensor_mpu.h"

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
const mpu9150_connect mpu9150_connections = {
    GPIOF,                // i2c sda port
    0,                    // i2c_sda_pad
    GPIOF,                // i2c_scl_port
    1,                    // i2c scl pad
    GPIOF,                // interrupt port
    13,                   // interrupt pad;
};

int main(void) {
    static const evhandler_t evhndl_main[]  = {
        sdc_insert_handler,
        sdc_remove_handler
    };
    struct EventListener     el0, el1;

    //struct lwipthread_opts   ip_opts;

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();
    led_init(&e407_led_cfg); //diagnostics

    psas_rtc_lld_init();

    psas_rtc_set_fc_boot_mark(&RTCD1); 

    /*
     * Activates the serial driver 6 and SDC driver 1 using default
     * configuration.
     */
    sdcStart(&SDCD1, NULL);

    /*
     * Activates the card insertion monitor.
     */
    sdc_tmr_init(&SDCD1);

    usbSerialShellStart(commands);


    mpu9150_start(&I2CD2);
    mpl3115a2_start(&I2CD2);

    /*
     * I2C2 I/O pins setup
     */
    palSetPadMode(mpu9150_connections.i2c_sda_port , mpu9150_connections.i2c_sda_pad,
            PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |PAL_STM32_PUDR_FLOATING );
    palSetPadMode(mpu9150_connections.i2c_scl_port, mpu9150_connections.i2c_scl_pad,
            PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST  | PAL_STM32_PUDR_FLOATING);

    palSetPad(mpu9150_connections.i2c_scl_port,  mpu9150_connections.i2c_scl_pad );

    // MPL pressure sensor
    chThdCreateStatic(waThread_mpl_int_1        , sizeof(waThread_mpl_int_1)        , NORMALPRIO    , Thread_mpl_int_1       , NULL);

    // MPU 6 axis IMU sensor
    chThdCreateStatic(waThread_mpu9150_int      , sizeof(waThread_mpu9150_int)      , NORMALPRIO    , Thread_mpu9150_int     , NULL);

    chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, SENSOR_LWIP);
    /*    chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
     *    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);
     */

    //chThdCreateStatic(wa_sdlog_thread           , sizeof(wa_sdlog_thread)           , NORMALPRIO    , sdlog_thread           , NULL);

    chEvtRegister(&sdc_inserted_event,   &el0, 0);
    chEvtRegister(&sdc_removed_event,    &el1, 1);

    // It is possible the card is already inserted. Check now by calling insert handler directly.
    sdc_insert_handler(0);

    while (TRUE) {
       chEvtDispatch(evhndl_main, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(50)));
    }
}

//! @}
