#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chrtclib.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "net_addrs.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "rnet_cmd_interp.h"
#include "BQ24725.h"
#include "BQ3060.h"

#include "rnh_shell.h"
#include "KS8999.h"
#include "RNHPort.h"

static int battery_socket;
static int port_socket;
static EVENTSOURCE_DECL(ACOK);

static const struct led * LED_ACOK = &BLUE;

/* RCI commands
 *
 */

static const char ARM[]     = "#YOLO";
static const char SAFE[]    = "#SAFE";
static const char VERSION[] = "#VERS";
static const char TIME[]    = "#TIME";
static const char PWR_STAT[]= "#POWR";

void cmd_time(struct RCICmdData * rci_data, void * user_data UNUSED){
    uint64_t time_ns = rtcGetTimeUnixUsec(&RTCD1) * 1000;

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

/* Hardware handling callbacks
 *
 */

void sleep(void){

}

static void ACOK_cb(EXTDriver *extp UNUSED, expchannel_t channel UNUSED) {
    if(BQ24725_ACOK()){
        ledOn(LED_ACOK);
        chSysLockFromIsr();
        chEvtBroadcastI(&ACOK);
        chSysUnlockFromIsr();
        //if low power mode is set
        //KS8999_enable(TRUE);
    }else{
    	ledOff(LED_ACOK);
        //if low power mode is set
        //KS8999_enable(FALSE);
        //sleep();
    }
}

void BQ24725_SetCharge(eventid_t id UNUSED){
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

static void BQ3060_SendData(eventid_t id UNUSED){
    struct BQ3060Data data;
    uint16_t buffer[13];
    BQ3060_get_data(&data);
    buffer[0] = htons(data.Temperature);
    buffer[1] = htons(data.TS1Temperature);
    buffer[2] = htons(data.TS2Temperature);
    buffer[3] = htons(data.TempRange);
    buffer[4] = htons(data.Voltage);
    buffer[5] = htons(data.Current);
    buffer[6] = htons(data.AverageCurrent);
    buffer[7] = htons(data.CellVoltage1);
    buffer[8] = htons(data.CellVoltage2);
    buffer[9] = htons(data.CellVoltage3);
    buffer[10] = htons(data.CellVoltage4);
    buffer[11] = htons(data.PackVoltage);
    buffer[12] = htons(data.AverageVoltage);
    write(battery_socket, buffer, sizeof(buffer));
}

static void portCurrent_SendData(eventid_t id UNUSED){
    struct rnhPortCurrent sample;
    uint16_t buffer[8];
    rnhPortGetCurrentData(&sample);
    buffer[0] = htons(sample.current[0]);
    buffer[1] = htons(sample.current[1]);
    buffer[2] = htons(sample.current[2]);
    buffer[3] = htons(sample.current[3]);
    buffer[4] = htons(sample.current[4]);
    buffer[5] = htons(sample.current[5]);
    buffer[6] = htons(sample.current[6]);
    buffer[7] = htons(sample.current[7]);
    write(port_socket, buffer, sizeof(buffer));
}

void main(void) {
    // Start Chibios
    halInit();
    chSysInit();

    // Start Diagnostics
    ledStart(NULL);
    rnh_shell_start();

    // Configuration
    static struct BQ24725Config BQConf = {
            .ACOK = {GPIOD, GPIO_D0_BQ24_ACOK},
            .ACOK_cb = ACOK_cb,
            .I2CD = &I2CD1
    };
    static struct BQ3060Config rnh3060conf = {
            .I2CD = &I2CD1
    };
    static struct RCIConfig conf;
    conf.commands = (struct RCICommand[]){
            {TIME, cmd_time, NULL},
            RCI_CMD_PORT,
            {NULL}
    };
    conf.address = RNH_RCI_ADDR,

    //Init hardware
    BQ24725Start(&BQConf);
    BQ3060Start(&rnh3060conf);
    KS8999Start();
    rnhPortStart();

    lwipThreadStart(RNH_LWIP);
    RCICreate(&conf);

    // Set up sockets
    battery_socket = get_udp_socket(RNH_BATTERY_ADDR);
    chDbgAssert(battery_socket >=0, "Battery socket failed", NULL);
    port_socket = get_udp_socket(RNH_PORT_ADDR);
    chDbgAssert(port_socket >=0, DBG_PREFIX"Port socket socket failed", NULL);

    connect(battery_socket, FC_ADDR, sizeof(struct sockaddr));
    connect(port_socket, FC_ADDR, sizeof(struct sockaddr));

    // Start charging if possible
    if(BQ24725_ACOK()){
        ledOn(LED_ACOK);
        BQ24725_SetCharge(0);
    }

    // Set up event system
    struct EventListener batt0, batt1, port0;
    chEvtRegister(&ACOK, &batt0, 0);
    chEvtRegister(&BQ3060_data_ready, &batt1, 1);
    chEvtRegister(&rnhPortCurrent, &port0, 2);
    const evhandler_t evhndl[] = {
        BQ24725_SetCharge,
        BQ3060_SendData,
        portCurrent_SendData
    };

    while (TRUE) {
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
    }
}
