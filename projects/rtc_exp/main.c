/*! \file main.c
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp RTC experiments
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
#include "rtc_lld.h"

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "board.h"

#include "device_net.h"
#include "fc_net.h"

#include "main.h"

/* see cmddetail.c for testing. Use 'date' in shell */

static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"date",  cmd_date},
    {"phy", cmd_phy},
    {NULL, NULL}
};

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
    static const evhandler_t evhndl_main[]       = {
        extdetail_WKUP_button_handler
    };
    struct EventListener     el0;

    /*struct lwipthread_opts   ip_opts;*/

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();
    extdetail_init();

    /*!
     * GPIO Pins for generating pulses at data input detect and data output send.
     * Used for measuring latency timing of data
     *
     * \sa board.h
     */
    //palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
    //palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    //palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
    //palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );

    usbSerialShellStart(commands);

    iwdg_begin();

    /*! Activates the EXT driver 1. */
    //extStart(&EXTD1, &extcfg);

    chThdCreateStatic(waThread_indwatchdog      , sizeof(waThread_indwatchdog)      , NORMALPRIO    , Thread_indwatchdog     , NULL);

    chThdSleepMilliseconds(2000);
    /*BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;*/

    /* DONE: Check cal on oscilloscope on RTC_AF1. 64hz */
    psas_rtc_lld_init();
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
 *
 */
     chEvtRegister(&extdetail_wkup_event, &el0, 0);

    while (TRUE) {
        chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
    }
}


//! @}
