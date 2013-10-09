//#include <stdbool.h>
//#include <stdlib.h>
#include "ch.h"
#include "hal.h"
//#include <lwip/ip_addr.h>

//#include "lwipopts.h"
//#include "lwipthread.h"

//#include "chprintf.h"
//#include "shell.h"

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
//static const ShellCommand commands[] = {
//#if DEBUG_KSZ
//		{"ksz_nodes_en_n", cmd_ksz_nodes_en_n},
//		{"ksz_n1en_n", cmd_ksz_n1en_n},
//		{"ksz_rst_n", cmd_ksz_rst_n},
//		{"ksz_pwr", cmd_ksz_pwr},
//#endif
//		{"show"    , cmd_show},
//		{"mem"    , cmd_mem},
//		{"threads", cmd_threads},
//		{NULL, NULL}
//};
//
//static const ShellConfig shell_cfg1 = {
//		(BaseSequentialStream *)&SD1,
//		commands
//};
//

static WORKING_AREA(led_area, 64);
/*! \brief Green LED blinker thread
 */
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
//
//void debug_msg_lwip(char* msg) {
//    BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;
//
//    chprintf(chp, "%s\r\n", msg);
//    chprintf(chp, "%d\r\n",  LWIP_SEND_TIMEOUT  );
//
//}
//
//
//
//void init_rnet(void) {
//	palSetPad(GPIOD, GPIO_D14_KSZ_EN);
//	palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
//        palClearPad(GPIOD, GPIO_D14_KSZ_EN);
//	chThdSleepMilliseconds(200);
//	palSetPad(GPIOD, GPIO_D14_KSZ_EN);     // enable pwr
//// palClearPad(GPIOD, GPIO_D13_RGB_R);
//	chThdSleepMilliseconds(200);
//	// Turn on clock from HSE -> PC9 function MCO2 see board file
//	RCC->CFGR |=  (1<<31);      // MCO0
//	RCC->CFGR &= ~(1<<29);
//
////	RCC->CFGR &=  ~(0b11<<22);      // MCO1
////	RCC->CFGR |=  (0b10<<21);
////
////	RCC->CFGR &= ~(0b111<<26); // clear MCO1 prescaler
////	RCC->CFGR |= (0b???<<26);  // set MCO1 prescaler
//// Timer 1 channel one instead?
//
//	palSetPad(GPIOD, GPIO_D4_ETH_N_RST);   // disable reset
//// palClearPad(GPIOD, GPIO_D11_RGB_B);
//	chThdSleepMilliseconds(500);
//}

/*
 * Application entry point.
 */

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
//	static Thread            *shelltp       = NULL;
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

    chThdCreateStatic(led_area, sizeof(led_area), NORMALPRIO, led, NULL);

    chThdSleep(TIME_INFINITE);
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
//	led_init();
//        init_rnet();
//
//	// start the serial port
//	sdStart(&SD1, NULL);
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

