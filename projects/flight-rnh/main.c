#include "ch.h"
#include "hal.h"
#include "rnh_shell.h"

//#include <lwip/ip_addr.h>

//#include "lwipopts.h"
//#include "lwipthread.h"

// Local header files
//#include "data_udp.h"
//#include "device_net.h"
//#include "fc_net.h"

static WORKING_AREA(led_area, 64);

__attribute__((noreturn))
static void led(void *arg __attribute__ ((unused))) {
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
        chThdSleepMilliseconds(50);
    }

    while (TRUE) {
        chThdSleepMilliseconds(led_wait_time);
        palTogglePad(GPIOD, GPIO_D12_RGB_G);
    }
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


void main(void) {
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

    chThdCreateStatic(led_area, sizeof(led_area), NORMALPRIO, (tfunc_t)led, NULL);

    //main should never return
    while (TRUE) {
        rnh_shell_start();
        chThdSleep(1);
    }
}

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

