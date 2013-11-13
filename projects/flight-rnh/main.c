#include "ch.h"
#include "hal.h"
#include "rnh_shell.h"
#include "BQ24725.h"
#include "KS8999.h"

#include "lwip/ip_addr.h"
#include "lwipopts.h"
#include "lwipthread.h"

// Local header files
//#include "data_udp.h"
//#include "device_net.h"
//#include "fc_net.h"
#define         RNET_A_IP_ADDR(p)         IP4_ADDR(p, 10, 0, 0,   5);
#define         RNET_A_GATEWAY(p)         IP4_ADDR(p, 0,  0, 0,   0  );
#define         RNET_A_NETMASK(p)         IP4_ADDR(p, 255, 255, 255, 0  );
#define         RNET_A_IP_ADDR_STRING     "10.0.0.5"
#define         RNET_A_LISTEN_PORT        35003
#define         RNET_A_TX_PORT            35004
#define         RNET_A_MAC_ADDRESS        {0xE6, 0x10, 0x20, 0x30, 0x40, 0xaa}

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


void BQ24725_init(void){
    BQ24725_charge_options BQ24725_rocket_init = {
                .ACOK_deglitch_time = t150ms,
                .WATCHDOG_timer = disabled,
                .BAT_depletion_threshold = FT70_97pct,
                .EMI_sw_freq_adj = dec18pct,
                .EMI_sw_freq_adj_en = sw_freq_adj_disable,
                .IFAULT_HI_threshold = l700mV,
                .LEARN_en = LEARN_disable,
                .IOUT = adapter_current,
                .ACOC_threshold = l1_66X,
                .charge_inhibit = charge_enable
            };
            BQ24725_SetChargeCurrent(0x400);
            BQ24725_SetChargeVoltage(0x41A0);
            BQ24725_SetInputCurrent(0x0A00);
            BQ24725_SetChargeOption(&BQ24725_rocket_init);
}

void sleep(void){

}

void main(void) {

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

    chThdCreateStatic(led_area, sizeof(led_area), NORMALPRIO, (tfunc_t)led, NULL);
    BQ24725_init();
    KS8999_init();

    struct lwipthread_opts ip_opts;
	struct ip_addr ip, gateway, netmask;
    uint8_t RNET_macAddress[6] = RNET_A_MAC_ADDRESS;

	RNET_A_IP_ADDR(&ip);
	RNET_A_GATEWAY(&gateway);
	RNET_A_NETMASK(&netmask);

	ip_opts.macaddress = RNET_macAddress;
	ip_opts.address    = ip.addr;
	ip_opts.netmask    = netmask.addr;
	ip_opts.gateway    = gateway.addr;
//
	chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, &ip_opts);
//	chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
//	chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);


    //main should never return
    while (TRUE) {
        rnh_shell_start();
        chThdSleep(1);
    }
}
