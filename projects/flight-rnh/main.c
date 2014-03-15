#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_led.h"
#include "utils_rtc.h"
#include "rnet_cmd_interp.h"
#include "BQ24725.h"
#include "BQ3060.h"

#include "rnh_shell.h"
#include "KS8999.h"
#include "RNH.h"

#define UNUSED __attribute__((unused))

static const char ARM[]     = "#YOLO";
static const char SAFE[]    = "#SAFE";
static const char PORT[]    = "#PORT";
static const char VERSION[] = "#VERS";
static const char TIME[]    = "#TIME";
static const char PWR_STAT[]= "#POWR";

void cmd_port(struct RCICmdData * rci_data, void * user_data UNUSED){
    if(rci_data->cmd_len < 2){
        return; //fixme return Error
    }
    RNH_action action = rci_data->cmd_data[0];
    int port_mask = rci_data->cmd_data[1];

    rci_data->return_data[0] = RNH_power(port_mask, action);
    rci_data->return_len = 1;
}

void cmd_time(struct RCICmdData * rci_data, void * user_data UNUSED){
    RTCTime timespec;
    rtcGetTime(&RTCD1, &timespec);

    uint64_t time_ns = rtc_to_ns(&timespec);

    rci_data->return_data[0] = time_ns & (0xFF << 7) >> 7;
    rci_data->return_data[1] = time_ns & (0xFF << 6) >> 6;
    rci_data->return_data[2] = time_ns & (0xFF << 5) >> 5;
    rci_data->return_data[3] = time_ns & (0xFF << 4) >> 4;
    rci_data->return_data[4] = time_ns & (0xFF << 3) >> 3;
    rci_data->return_data[5] = time_ns & (0xFF << 2) >> 2;
    rci_data->return_data[6] = time_ns & (0xFF << 1) >> 1;
    rci_data->return_data[7] = time_ns & (0xFF << 0) >> 0;

    rci_data->return_len = 8;
}

void eth_start(void){
    static struct RCICommand cmds[] = {
            {TIME, cmd_time, NULL},
            {PORT, cmd_port, NULL},
            {NULL, NULL, NULL}
    };

    static struct RCIConfig conf;
    conf.address = RNH_LISTEN_ADDR;
    conf.commands = cmds;

    chThdCreateStatic(wa_lwip_thread, sizeof(wa_lwip_thread), NORMALPRIO + 2, lwip_thread, RNH_LWIP);
    RCICreate(&conf);
}

void sleep(void){

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

static void bqst(eventid_t id UNUSED){
    BQ24725_start();
}

static EventSource bqst_event;

static void ACOK_cb(EXTDriver *extp UNUSED, expchannel_t channel UNUSED) {
    if(BQ24725_ACOK()){

        palClearPad(GPIOD, GPIO_D11_RGB_B);
        chSysLockFromIsr();
        chEvtBroadcastI(&bqst_event);
        chSysUnlockFromIsr();
        //if low power mode is set
//        KS8999_enable(TRUE);
    }else{
        palSetPad(GPIOD, GPIO_D11_RGB_B);
        //if low power mode is set
//        sleep();
//        KS8999_enable(FALSE);
    }
}

void main(void) {
    halInit();
    chSysInit();
    led_init(&rnh_led_cfg); //diagnostic LED blinker

    //Set up events
    chEvtInit(&bqst_event);
    struct EventListener el0;
    chEvtRegister(&bqst_event, &el0, 0);
    const evhandler_t evhndl[] = {
        bqst
    };

    //Init hardware
    struct BQ24725Config BQConf = {
            .ACOK = {GPIOD, GPIO_D0_BQ24_ACOK},
            .ACOK_cb = ACOK_cb,
            .I2CD = &I2CD1
    };
    BQ24725_init(&BQConf);
    BQ3060_init(&rnh3060conf);
    if(BQ24725_ACOK()){
        palClearPad(GPIOD, GPIO_D11_RGB_B);
        BQ24725_start();
    }
    KS8999_init();
    eth_start();

    rnh_shell_start();
    while (TRUE) {
        chEvtDispatch(evhndl, chEvtWaitAll(ALL_EVENTS));
    }
}
