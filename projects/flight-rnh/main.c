#include <string.h>
#include "ch.h"
#include "hal.h"
#include "rnh_shell.h"
#include "BQ24725.h"
#include "KS8999.h"
#include "RNH.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "rnet_cmd_interp.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipopts.h"
#include "lwipthread.h"

#define UNUSED __attribute__((unused))
#define NORETURN __attribute__((noreturn))

#include "chprintf.h"

static WORKING_AREA(led_area, 64);
NORETURN static void led(void *arg UNUSED) {
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



msg_t ethernet_commands(void * arg UNUSED){
    BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;
    int s = get_udp_socket(RNH_LISTEN_ADDR);

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
    chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, RNH_LWIP);
    chThdCreateStatic(wa_ethernet_commands , sizeof(wa_ethernet_commands) , NORMALPRIO , ethernet_commands, NULL);
}

void sleep(void){

}

static void ACOK_cb(EXTDriver *extp UNUSED, expchannel_t channel UNUSED) {

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
