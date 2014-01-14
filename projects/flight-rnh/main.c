#include <string.h>
#include "ch.h"
#include "hal.h"
#include "rnh_shell.h"
#include "BQ24725.h"
#include "KS8999.h"
#include "RNH.h"
#include "device_net.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "chprintf.h"

#define         RNET_A_IP_ADDR(p)         IP4_ADDR(p, 10, 0, 0,   5);
#define         RNET_A_GATEWAY(p)         IP4_ADDR(p, 0,  0, 0,   0  );
#define         RNET_A_NETMASK(p)         IP4_ADDR(p, 255, 255, 255, 0  );
#define         RNET_A_IP_ADDR_STRING     "10.0.0.5"
#define         RNET_A_LISTEN_PORT        36100
#define         RNET_A_TX_PORT            36101
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
 * eth terminal
 * logic (turn on/off port, etc)
 * Logger
 *
 */

#define ETHERNET_COMMANDS_STACK_SIZE      1024
WORKING_AREA(wa_ethernet_commands, ETHERNET_COMMANDS_STACK_SIZE);

char ARM[]     = "#YOLO";
char SAFE[]    = "#SAFE";
char PIN_ON[]  = "#ON_P";
char PIN_OFF[] = "#FF_P";
char VERSION[] = "#VERS";
char TIME[]    = "#TIME";
char PWR_STAT[]= "#POWR";

#define COMPARE_BUFFER_TO_CMD(a, b, len)\
    !strncmp((char*)a, b, sizeof(b) > len? len: sizeof(b))



msg_t ethernet_commands(void * arg __attribute__((unused))){
    BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;
    struct sockaddr_in rnh_in;
    memset(&rnh_in, 0, sizeof(struct sockaddr_in));
    rnh_in.sin_family = AF_INET,
    rnh_in.sin_port = htons(RNET_A_LISTEN_PORT);
    inet_aton(RNET_A_IP_ADDR_STRING, &rnh_in.sin_addr);

    int s = socket(AF_INET,  SOCK_DGRAM, 0);
    if(s < 0){
        return -1;
    }

    if(bind(s, (struct sockaddr * )&rnh_in, sizeof(rnh_in)) < 0){
        return -2;
    }

    char msg[50]; //TODO: non-arbitrary size
    memset(msg, 0, sizeof(msg));
    int ret;
    unsigned int len;
    while(TRUE) {
        ret = recv(s, msg, sizeof(msg), 0);
        if(ret < 0){
            return -3;
        }
        len=ret;

        if(COMPARE_BUFFER_TO_CMD(msg, ARM, len)){

        } else if(COMPARE_BUFFER_TO_CMD(msg, SAFE, len)){

        } else if(COMPARE_BUFFER_TO_CMD(msg, PIN_ON, 4)){
            chprintf(chp, "port: %d on\r\n", msg[5]);
            int port_mask = msg[5];
            RNH_power(port_mask, RNH_PORT_ON);
        } else if(COMPARE_BUFFER_TO_CMD(msg, PIN_OFF, 4)){
            chprintf(chp, "port: %d off \r\n", msg[5]);
            int port_mask = msg[5];
            RNH_power(port_mask, RNH_PORT_OFF);
        } else if(COMPARE_BUFFER_TO_CMD(msg, VERSION, len)){

        } else if(COMPARE_BUFFER_TO_CMD(msg, TIME, len)){

        } else if(COMPARE_BUFFER_TO_CMD(msg, PWR_STAT, len)){

        }
    }
    return 0;
}

void BQ24725_start(void){
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

void eth_start(void){
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

    chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, &ip_opts);
    chThdCreateStatic(wa_ethernet_commands , sizeof(wa_ethernet_commands) , NORMALPRIO , ethernet_commands, NULL);
}

void sleep(void){

}

static void ACOK_cb(EXTDriver *extp __attribute__ ((unused)),
                    expchannel_t channel __attribute__ ((unused))) {

    if(BQ24725_ACOK()){
        KS8999_enable(TRUE);
    }else{
        RNH_power(RNH_PORT_ALL, RNH_PORT_OFF);
        KS8999_enable(FALSE);
    }
}

void main(void) {
    halInit();
    chSysInit();

    //start the LED blinker thread
    chThdCreateStatic(led_area, sizeof(led_area), NORMALPRIO, (tfunc_t)led, NULL);

    //Init hardware
    if(BQ24725_ACOK()){
        BQ24725_start();
        KS8999_init();
        eth_start();
    }

    //Start ACOK power events
    EXTConfig bqextcfg;
    memset(&bqextcfg, 0, sizeof(bqextcfg));
    bqextcfg.channels[0].mode = EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOD;
    bqextcfg.channels[0].cb = &ACOK_cb;
    extStart(&EXTD1, &bqextcfg);

    //main should never return
    while (TRUE) {
        rnh_shell_start();
        chThdSleep(1);
    }
}
