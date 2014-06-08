#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chrtclib.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "net_addrs.h"
#include "rnet_cmd_interp.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "BQ24725.h"
#include "BQ3060.h"

#include "rnh_shell.h"
#include "KS8999.h"
#include "RNHPort.h"

static struct SeqSocket battery_socket = DECL_SEQ_SOCKET(13*2);
static struct SeqSocket port_socket = DECL_SEQ_SOCKET(8*2);

static EVENTSOURCE_DECL(ACOK);

static const struct led * LED_ACOK = &BLUE;

/* RCI commands
 *
 */

static const char ARM[]     = "#YOLO";
static const char SAFE[]    = "#SAFE";
static const char TIME[]    = "#TIME";

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
    BQ3060_get_data(&data);
    battery_socket.buffer[0] = htons(data.Temperature);
    battery_socket.buffer[1] = htons(data.TS1Temperature);
    battery_socket.buffer[2] = htons(data.TS2Temperature);
    battery_socket.buffer[3] = htons(data.TempRange);
    battery_socket.buffer[4] = htons(data.Voltage);
    battery_socket.buffer[5] = htons(data.Current);
    battery_socket.buffer[6] = htons(data.AverageCurrent);
    battery_socket.buffer[7] = htons(data.CellVoltage1);
    battery_socket.buffer[8] = htons(data.CellVoltage2);
    battery_socket.buffer[9] = htons(data.CellVoltage3);
    battery_socket.buffer[10] = htons(data.CellVoltage4);
    battery_socket.buffer[11] = htons(data.PackVoltage);
    battery_socket.buffer[12] = htons(data.AverageVoltage);
    seq_write(&battery_socket, 13*2);
}

static void portCurrent_SendData(eventid_t id UNUSED){
    struct rnhPortCurrent sample;
    rnhPortGetCurrentData(&sample);
    port_socket.buffer[0] = htons(sample.current[0]);
    port_socket.buffer[1] = htons(sample.current[1]);
    port_socket.buffer[2] = htons(sample.current[2]);
    port_socket.buffer[3] = htons(sample.current[3]);
    port_socket.buffer[4] = htons(BQ24725_IMON());
    port_socket.buffer[5] = htons(sample.current[5]);
    port_socket.buffer[6] = htons(sample.current[6]);
    port_socket.buffer[7] = htons(sample.current[7]);
    seq_write(&port_socket, 8*2);
}

void main(void) {
    int s;

    // Start Chibios
    halInit();
    chSysInit();

    // Start Diagnostics
    ledStart(NULL);
    rnh_shell_start();

    // Configuration
    static I2CPins I2C1Pins = {
            .SDA = {GPIOB, GPIO_B7_I2C_SDA},
            .SCL = {GPIOB, GPIO_B6_I2C_SCL}
    };
    static struct BQ24725Config BQConf = {
            .ACOK = {GPIOD, GPIO_D0_BQ24_ACOK},
            .ACOK_cb = ACOK_cb,
            .I2CD = &I2CD1,
            .I2CP = &I2C1Pins,
            .ADCD = &ADCD3
    };
    static struct BQ3060Config rnh3060conf = {
            .I2CD = &I2CD1,
            .I2CP = &I2C1Pins
    };
    static struct RCIConfig conf;
    conf.commands = (struct RCICommand[]){
            {TIME, cmd_time, NULL},
            RCI_CMD_PORT,
            RCI_CMD_VERS,
            {NULL}
    };
    conf.address = RNH_RCI_ADDR;

    //Init hardware
    BQ24725Start(&BQConf);
    BQ3060Start(&rnh3060conf);
    KS8999Start();
    rnhPortStart();

    lwipThreadStart(RNH_LWIP);
    RCICreate(&conf);

    // Set up sockets
    s = get_udp_socket(RNH_BATTERY_ADDR);
    chDbgAssert(s >=0, "Battery socket failed", NULL);
    seq_socket_init(&battery_socket, s);

    s = get_udp_socket(RNH_PORT_ADDR);
    chDbgAssert(s >=0, DBG_PREFIX"Port socket socket failed", NULL);
    seq_socket_init(&port_socket, s);

    connect(battery_socket.socket, FC_ADDR, sizeof(struct sockaddr));
    connect(port_socket.socket, FC_ADDR, sizeof(struct sockaddr));

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
