/*! \file main.c
 *
 * This is specific to the Olimex stm32-e407 board.
 *
 * Test writing data to the sd card and develop the psas_sdlog interface
 */

/*!
 * \defgroup mainapp FATFS experiments
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "iwdg_lld.h"
#include "usbdetail.h"
#include "extdetail.h"
#include "cmddetail.h"
#include "psas_rtc.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "board.h"

#include "device_net.h"
#include "fc_net.h"

#include "psas_sdclog.h"
#include "sdcdetail.h"
#include "ff.h"

#include "main.h"

static const ShellCommand commands[] = {
    {"sdct",    cmd_sdct},
    {"mem",     cmd_mem},
    {"threads", cmd_threads},
    {"date",    cmd_date},
    {"phy",     cmd_phy},
    {"tree",    cmd_tree},
    {NULL,      NULL}
};

static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SDU_PSAS,
    commands
};

static WORKING_AREA(waThread_blinker, 64);
/*! \brief Green LED blinker thread
*/
static msg_t Thread_blinker(void *arg) {
    (void)arg;
    chRegSetThreadName("blinker");
    while (TRUE) {
        palTogglePad(GPIOC, GPIOC_LED);
        chThdSleepMilliseconds(fs_ready ? 125 : 500);
    }
    return -1;
}

static WORKING_AREA(waThread_indwatchdog, 64);
/*! \brief  Watchdog thread
*/
static msg_t Thread_indwatchdog(void *arg) {
    (void)arg;

    chRegSetThreadName("iwatchdog");
    while (TRUE) {
        iwdg_lld_reload();
        chThdSleepMilliseconds(250);
    }
    return -1;
}

int main(void) {
    static Thread            *shelltp       = NULL;
    static const evhandler_t evhndl_main[]  = {
        extdetail_WKUP_button_handler,
        sdc_insert_handler,
        sdc_remove_handler
    };
    struct EventListener     el0, el1, el2;

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

    psas_rtc_lld_init();

    RTCTime   psas_time;
    struct    tm timp;
    time_t    unix_time;
    psas_rtc_lld_get_time(&RTCD1, &psas_time);
    psas_stm32_rtc_bcd2tm(&timp, &psas_time);
    unix_time = mktime(&timp);
    psas_rtc_set_fc_boot_mark(unix_time); 

    extdetail_init();

    palSetPad(GPIOC, GPIOC_LED);

    /*!
     * GPIO Pins for generating pulses at data input detect and data output send.
     * Used for measuring latency timing of data
     *
     * \sa board.h
     */
    palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
    palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
    palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );

    /*!
     * Initializes a serial-over-USB CDC driver.
     */
    sduObjectInit(&SDU_PSAS);
    sduStart(&SDU_PSAS, &serusbcfg);

    /*
     * Activates the serial driver 6 and SDC driver 1 using default
     * configuration.
     */
    sdStart(&SD6, NULL);
    sdcStart(&SDCD1, NULL);

    /*
     * Activates the card insertion monitor.
     */
    sdc_tmr_init(&SDCD1);

    /*!
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    shellInit();

    iwdg_begin();

    chThdSleepMilliseconds(300);

    /*! Activates the EXT driver 1. */
    extStart(&EXTD1, &extcfg);

    chThdCreateStatic(waThread_blinker          , sizeof(waThread_blinker)          , NORMALPRIO    , Thread_blinker         , NULL);
    chThdCreateStatic(waThread_indwatchdog      , sizeof(waThread_indwatchdog)      , NORMALPRIO    , Thread_indwatchdog     , NULL);

    /*
     *    static       uint8_t      IMU_macAddress[6]           = IMU_A_MAC_ADDRESS;
     *    struct       ip_addr      ip, gateway, netmask;
     *    IMU_A_IP_ADDR(&ip);
     *    IMU_A_GATEWAY(&gateway);
     *    IMU_A_NETMASK(&netmask);
     *
     *    ip_opts.macaddress = IMU_macAddress;
     *    ip_opts.address    = ip.addr;
     *    ip_opts.netmask    = netmask.addr;
     *    ip_opts.gateway    = gateway.addr;
     *
     *    chThdCreateStatic(wa_lwip_thread            , sizeof(wa_lwip_thread)            , NORMALPRIO + 2, lwip_thread            , &ip_opts);
     *    chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
     *    chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);
     */

    chThdCreateStatic(wa_sdlog_thread           , sizeof(wa_sdlog_thread)           , NORMALPRIO    , sdlog_thread           , NULL);

    chEvtRegister(&extdetail_wkup_event, &el0, 0);
    chEvtRegister(&sdc_inserted_event,   &el1, 1);
    chEvtRegister(&sdc_removed_event,    &el2, 2);

    // It is possible the card is already inserted. Check now by calling insert handler directly.
    sdc_insert_handler(0);

    while (TRUE) {
        if (!shelltp && (SDU_PSAS.config->usbp->state == USB_ACTIVE))
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        else if (chThdTerminated(shelltp)) {
            chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
            shelltp = NULL;           /* Triggers spawning of a new shell.        */
        }
        chEvtDispatch(evhndl_main, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(50)));
    }
}


//! @}
