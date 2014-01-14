/*! \file main.c
 *
 *  PWM controlled from FC
 *
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

#include <lwip/ip_addr.h>

#include "data_udp.h"
#include "lwipopts.h"
#include "lwipthread.h"
#include "pwm.h"
#include "pwm_config.h"

#include "device_net.h"
#include "fc_net.h"

#include "main.h"

#define 		DEBUG_PWM 				0

#define GPIOF_GREEN_LED             2
#define GPIOF_RED_LED               3
#define GPIOF_BLUE_LED              14

static const ShellCommand commands[] = {
        {"mem", cmd_mem},
        {"threads", cmd_threads},
        {NULL, NULL}
};


BaseSequentialStream * chp = NULL;

static void led_init(void) {

    palClearPad(GPIOF, GPIOF_BLUE_LED);
    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_GREEN_LED);

    int i = 0;
    for(i=0; i<5; ++i) {
        palSetPad(GPIOF, GPIOF_RED_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_RED_LED);
        palSetPad(GPIOF, GPIOF_BLUE_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_BLUE_LED);
        palSetPad(GPIOF, GPIOF_GREEN_LED);
        chThdSleepMilliseconds(150);
        palClearPad(GPIOF, GPIOF_GREEN_LED);
    }
}

static WORKING_AREA(waThread_launch_detect, 1024);
static msg_t Thread_launch_detect(void *arg) {
    (void)arg;
    static const evhandler_t evhndl_launch_det[] = {
            extdetail_launch_detect_handler
    };
    struct EventListener     evl_launch_det;

    chRegSetThreadName("launch_detect");
    launch_detect_init();

    chEvtRegister(&extdetail_launch_detect_event,      &evl_launch_det,         0);

    while (TRUE) {
        chEvtDispatch(evhndl_launch_det, chEvtWaitOneTimeout((EVENT_MASK(0)), US2ST(50)));
    }
    return -1;
}

#if DEBUG_PWM
static WORKING_AREA(waThread_pwmtest, 512);
static msg_t Thread_pwmtest(void *arg) {
    (void)arg;
    chRegSetThreadName("pwmtest");

    //    uint32_t i = 0;
    //
    chThdSleepMilliseconds(1000);

    chprintf(chp, "%d %d %d\n", pwm_us_to_ticks(1050), pwm_us_to_ticks(1500), pwm_us_to_ticks(1090));

    while(1) {
        uint32_t pulse = 0;

        //     chprintf(chp, ".");
        //        pwm_set_pulse_width_ticks(pwm_us_to_ticks(333));
        //        chThdSleepMilliseconds(200);
        //
        //        pwm_set_pulse_width_ticks(pwm_us_to_ticks(1050));
        //        chThdSleepMilliseconds(200);
        //
        //        pwm_set_pulse_width_ticks(pwm_us_to_ticks(1900));
        //        chThdSleepMilliseconds(200);
        //
        //        pwmDisableChannel(&PWMD4, 3);
        //chThdSleepMilliseconds(250);
        for (pulse = 1050; pulse <= 1900; pulse += 10) {
            pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
            chThdSleepMilliseconds(50);
        }
        pwmDisableChannel(&PWMD4, 3);
        chThdSleepMilliseconds(2500);

        for (pulse = pulse; pulse > 1050; pulse -= 10) {
        	pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
        	chThdSleepMilliseconds(50);
        }
//

    }
    return -1;
}
#endif

/*blinker thread vars and functions*/
static WORKING_AREA(waThread_blinker, 128);
static msg_t Thread_blinker(void *arg) {
    (void)arg;
    chRegSetThreadName("blinker");

    PIN_MODE_OUTPUT(GPIOF_BLUE_LED);
    palClearPad(GPIOF, GPIOF_GREEN_LED);
    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_BLUE_LED);
    while (TRUE) {
        palTogglePad(GPIOC, GPIOC_LED);
        palTogglePad(GPIOF, GPIOF_GREEN_LED);
        chThdSleepMilliseconds(500);
    }
    return -1;
}

/*watchdog timer thread vars and functions*/
static WORKING_AREA(waThread_indwatchdog, 128);
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

    struct lwipthread_opts   ip_opts;

    /* initialize HAL */
    halInit();
    chSysInit();

    extdetail_init();
    palSetPad(GPIOC, GPIOC_LED);

    palClearPad(GPIOF, GPIOF_RED_LED);
    palClearPad(GPIOF, GPIOF_BLUE_LED);
    palClearPad(GPIOF, GPIOF_GREEN_LED);


    usbSerialShellStart(commands);
    chp = getActiveUsbSerialStream();

    led_init();

    iwdg_begin();

    extStart(&EXTD1, &extcfg);

    chThdCreateStatic(waThread_blinker,      sizeof(waThread_blinker),      NORMALPRIO, Thread_blinker,      NULL);

    pwm_start();

    static       uint8_t      ROLL_CTL_macAddress[6]         = ROLL_CTL_MAC_ADDRESS;
    struct       ip_addr      ip, gateway, netmask;
    ROLL_CTL_IP_ADDR(&ip);
    ROLL_CTL_GATEWAY(&gateway);
    ROLL_CTL_NETMASK(&netmask);

    ip_opts.macaddress = ROLL_CTL_macAddress;
    ip_opts.address    = ip.addr;
    ip_opts.netmask    = netmask.addr;
    ip_opts.gateway    = gateway.addr;

    chThdCreateStatic(wa_lwip_thread              , sizeof(wa_lwip_thread)              , NORMALPRIO + 2, lwip_thread            , &ip_opts);
    //chThdCreateStatic(wa_data_udp_send_thread     , sizeof(wa_data_udp_send_thread)     , NORMALPRIO    , data_udp_send_thread   , NULL);
    chThdCreateStatic(wa_data_udp_receive_thread  , sizeof(wa_data_udp_receive_thread)  , NORMALPRIO    , data_udp_receive_thread, NULL);

    chThdCreateStatic(waThread_launch_detect      , sizeof(waThread_launch_detect)      , NORMALPRIO    , Thread_launch_detect   , NULL);

    chThdCreateStatic(waThread_indwatchdog        , sizeof(waThread_indwatchdog)        , NORMALPRIO    , Thread_indwatchdog     , NULL);

#if DEBUG_PWM
    chThdCreateStatic(waThread_pwmtest, sizeof(waThread_pwmtest), NORMALPRIO, Thread_pwmtest, NULL);
#endif

    chEvtRegister(&extdetail_wkup_event, &el0, 0);


    while (TRUE) {
        chEvtDispatch(evhndl_main, chEvtWaitOneTimeout((eventmask_t)1, MS2ST(500)));
    }
}
