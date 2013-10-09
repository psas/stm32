//#include <stdbool.h>
//#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"
//#include <lwip/ip_addr.h>

//#include "lwipopts.h"
//#include "lwipthread.h"




// Modified configuration files
//#include "board.h"

// Local header files
//#include "data_udp.h"
//#include "device_net.h"
//#include "fc_net.h"
//#include "cmddetail.h"

//#include "main.h"
//
///*! The goal of this code is to run the shell through the serial terminal
// * and not the usb subsystem. Connect an FTDI serial/usb connector to the
// * appropriate (in rocketnet hub land this is pa9 and pa10)
// * These are configured as alternate functions in the board.h file.
// *
// * See the rocketnet-hub schematic for pinout.
// *
// *     stm		rnethub      ftdi
// * RX: pa10     ?            tx
// * TX: pa9      ?            rx
// *
// * In the mcuconf.h file enable the proper serial subsystem:
// * #define STM32_SERIAL_USE_USART1             TRUE
// *
// * In the halconf.h enable the serial system
// * #define HAL_USE_SERIAL                      TRUE
// */
//
//

void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
    static const char *states[] = {THD_STATE_NAMES};
    Thread *tp;

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: threads\r\n");
        return;
    }
    chprintf(chp, "addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");
    tp = chRegFirstThread();
    do {
        chprintf(chp, "%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n",
                (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
                (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
        tp = chRegNextThread(tp);
    } while (tp != NULL);
}

void cmd_power(BaseSequentialStream *chp, int argc, char *argv[]) {
    // TODO: pwr <port ...> [on|off], port alias?
    if(argc < 1 || argc > 2){
        chprintf(chp, "Usage: pwr [on|off] <port>\r\n");
        return;
    }
    //Index 0 is left at a safe value instead of something like NULL
    uint32_t power[] = {GPIO_E4_NC, GPIO_E0_NODE1_N_EN, GPIO_E1_NODE2_N_EN,
                        GPIO_E2_NODE3_N_EN, GPIO_E3_NODE4_N_EN, GPIO_E4_NC,
                        GPIO_E5_NODE6_N_EN, GPIO_E6_NODE7_N_EN};
    uint32_t fault[] = {GPIO_E12_NC, GPIO_E8_NODE1_N_FLT, GPIO_E9_NODE2_N_FLT,
                        GPIO_E10_NODE3_N_FLT, GPIO_E11_NODE4_N_FLT, GPIO_E12_NC,
                        GPIO_E13_NODE6_N_FLT, GPIO_E14_NODE7_N_FLT};
    int port = 0;
    int port_i = argc - 1;
    if(!strcmp(argv[port_i], "1")){
        port = 1;
    }else if(!strcmp(argv[port_i], "2")){
        port = 2;
    }else if(!strcmp(argv[port_i], "3")){
        port = 3;
    }else if(!strcmp(argv[port_i], "4")){
        port = 4;
    }else if(!strcmp(argv[port_i], "6")){
        port = 6;
    }else if(!strcmp(argv[port_i], "7")){
        port = 7;
    }else if(!strcmp(argv[port_i], "all")){
        port = -1;
    }else {
        chprintf(chp, "Invalid port: %s. Valid ports are 1, 2, 3, 4, 6, 7, all\r\n", argv[port_i]);
        return;
    }

    int i;
    if(argc == 2){
        if(!strcmp(argv[0], "on")){
            if(port != -1){
                palClearPad(GPIOE, power[port]);
            }else{
                for(i = 1; i <= 7; ++i){
                    if(i == 5){
                        continue;
                    }
                    palClearPad(GPIOE, power[i]);
                }
            }
        }else if(!strcmp(argv[0], "off")){
            if(port != -1){
                palSetPad(GPIOE, power[port]);
            }else{
                for(i = 1; i <= 7; ++i){
                    if(i == 5){
                        continue;
                    }
                    palSetPad(GPIOE, power[i]);
                }
            }
        }else{
            chprintf(chp, "First argument must be on|off\r\n", argv[0]);
            return;
        }
    }else{
        if(port != -1){
            chprintf(chp, "%d: ", port);
            if(palReadPad(GPIOE, power[port])){
                chprintf(chp, "off, ");
            }else{
                chprintf(chp, "on, ");
            }
            if(palReadPad(GPIOE, fault[port])){
                chprintf(chp, "nominal\r\n");
            }else{
                chprintf(chp, "fault\r\n");
            }
        }else{
            for(i = 1; i <= 7; ++i){
                if(i == 5){
                    continue;
                }
                chprintf(chp, "%d: ", i);
                if(palReadPad(GPIOE, power[i])){
                    chprintf(chp, "off, ");
                }else{
                    chprintf(chp, "on, ");
                }
                if(palReadPad(GPIOE, fault[i])){
                    chprintf(chp, "nominal\r\n");
                }else{
                    chprintf(chp, "fault\r\n");
                }
            }
        }
    }
}

#define SHELL_WA_SIZE THD_WA_SIZE(2048)
static const ShellCommand commands[] = {
		{"threads", cmd_threads},
		{"pwr", cmd_power},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SD1,
		commands
};


static WORKING_AREA(led_area, 64);

static msg_t led(void *arg __attribute__ ((unused))) {
    const int led_wait_time = 500;

    chRegSetThreadName("LED");

    palClearPad(GPIOD, GPIO_D12_RGB_G);
    palClearPad(GPIOD, GPIO_D13_RGB_R);
    palClearPad(GPIOD, GPIO_D11_RGB_B);

    int i = 0;
    for(i=0; i<5; ++i) {
        palClearPad(GPIOD, GPIO_D12_RGB_G);
        chThdSleepMilliseconds(150);
        palSetPad(GPIOD, GPIO_D12_RGB_G);
        palClearPad(GPIOD, GPIO_D13_RGB_R);
        chThdSleepMilliseconds(150);
        palSetPad(GPIOD, GPIO_D13_RGB_R);
        palClearPad(GPIOD, GPIO_D11_RGB_B);
        chThdSleepMilliseconds(150);
        palSetPad(GPIOD, GPIO_D11_RGB_B);
        chThdSleepMilliseconds(150);
    }

    while (TRUE) {
        chThdSleepMilliseconds(led_wait_time);
        palTogglePad(GPIOD, GPIO_D12_RGB_G);
    }
    return -1;
}

void init_rnet(void) {
    palSetPad(GPIOD, GPIO_D14_KSZ_EN);
    palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
    palClearPad(GPIOD, GPIO_D14_KSZ_EN);
    chThdSleepMilliseconds(200);
    palSetPad(GPIOD, GPIO_D14_KSZ_EN);     // enable pwr
// palClearPad(GPIOD, GPIO_D13_RGB_R);
    chThdSleepMilliseconds(200);
    // Turn on clock from HSE -> PC9 function MCO2 see board file
    RCC->CFGR |=  (1<<31);      // MCO0
    RCC->CFGR &= ~(1<<29);

//  RCC->CFGR &=  ~(0b11<<22);      // MCO1
//  RCC->CFGR |=  (0b10<<21);
//
//  RCC->CFGR &= ~(0b111<<26); // clear MCO1 prescaler
//  RCC->CFGR |= (0b???<<26);  // set MCO1 prescaler
// Timer 1 channel one instead?

    palSetPad(GPIOD, GPIO_D4_ETH_N_RST);   // disable reset
// palClearPad(GPIOD, GPIO_D11_RGB_B);
    chThdSleepMilliseconds(500);
}


/*
 * Parts that we need:
 * BQ
 * KSZ
 * Serial/eth terminals
 * logic (turn on/off port, etc)
 * Logger
 * LED
 *
 */


int main(void) {
//	struct lwipthread_opts   ip_opts;
//
	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
    halInit();
    chSysInit();
//    init_rnet();

    chThdCreateStatic(led_area, sizeof(led_area), NORMALPRIO, led, NULL);

    sdStart(&SD1, NULL);
    shellInit();
    static Thread *shelltp = NULL;
    while (TRUE) {
        if (!shelltp)
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        else if (chThdTerminated(shelltp)) {
            chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
            shelltp = NULL;           /* Triggers spawning of a new shell.        */
        }
        chThdSleep(1);
    }
    // TODO: main should probably be void, but then we need to
    // disable a compiler warning
    return 0;
}
//	/*!
//		 * GPIO Pins for generating pulses at data input detect and data output send.
//		 * Used for measuring latency timing of data
//		 *
//		 * \sa board.h
//		 */
////		palClearPad(  TIMEOUTPUT_PORT, TIMEOUTPUT_PIN);
////		palSetPadMode(TIMEOUTPUT_PORT, TIMEOUTPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL);
////		palSetPad(    TIMEINPUT_PORT, TIMEINPUT_PIN);
////		palSetPadMode(TIMEINPUT_PORT, TIMEINPUT_PIN, PAL_MODE_OUTPUT_PUSHPULL );
//
//	// start the serial port
//
//
//	/*
//	 * Shell manager initialization.
//	 */
//	shellInit();
//
//	chThdCreateStatic(waThread_blinker  , sizeof(waThread_blinker)          , NORMALPRIO    , Thread_blinker         , NULL);
//	//chThdCreateStatic(waThread_25mhz    , sizeof(waThread_25mhz)            , NORMALPRIO    , Thread_25mhz           , NULL);
//
//	static       uint8_t      RNET_macAddress[6]           = RNET_A_MAC_ADDRESS;
//	struct       ip_addr      ip, gateway, netmask;
//	RNET_A_IP_ADDR(&ip);
//	RNET_A_GATEWAY(&gateway);
//	RNET_A_NETMASK(&netmask);
//
//	ip_opts.macaddress = RNET_macAddress;
//	ip_opts.address    = ip.addr;
//	ip_opts.netmask    = netmask.addr;
//	ip_opts.gateway    = gateway.addr;
//
//	chThdCreateStatic(wa_lwip_thread            , sizeof(wa_lwip_thread)            , NORMALPRIO + 2, lwip_thread            , &ip_opts);
//	chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
//	chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);
//
//
//	/*
//	 * Normal main() thread activity, in this demo it enables and disables the
//	 * button EXT channel using 5 seconds intervals.
//	 */
//
//	while (true) {
//		if (!shelltp )
//			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
//		else if (chThdTerminated(shelltp)) {
//			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
//			shelltp = NULL;           /* Triggers spawning of a new shell.        */
//		}
//		chThdSleepMilliseconds(1000);
//	}
//

