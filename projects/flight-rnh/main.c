#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chrtclib.h"
#include "evtimer.h"

#include "net_addrs.h"
#include "rci.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "utils_hal.h"
#include "BQ24725.h"
#include "BQ3060.h"
#include "KS8999.h"
#include "RNHPort.h"
#ifdef FLIGHT
#include "iwdg.h"
#endif
static struct SeqSocket battery_socket = DECL_SEQ_SOCKET(13*2);
static struct SeqSocket port_socket = DECL_SEQ_SOCKET(8*2);
static struct SeqSocket alarm_socket = DECL_SEQ_SOCKET(2*3);
static struct SeqSocket umbdet_socket = DECL_SEQ_SOCKET(1);

static EVENTSOURCE_DECL(UMBDET);
static EVENTSOURCE_DECL(ACOK);
static const struct led * LED_ACOK = &BLUE;

/* Pin features */
static int umbilical(void){
	return !palReadPad(GPIOC, GPIO_C13_UMB_DETECT);
}
static int rrdyStatus(void){
	return !palReadPad(GPIOD, GPIO_D2_N_ROCKET_READY);
}
static void rrdySet(int state){
	if(state){
		palClearPad(GPIOD, GPIO_D2_N_ROCKET_READY);
	} else {
		palSetPad(GPIOD, GPIO_D2_N_ROCKET_READY);
	}
}

/* RCI commands */
static const char ARM[]  = "#YOLO";
static const char SAFE[] = "#SAFE";
static const char TIME[] = "#TIME";
static const char RRDY[] = "#RRDY";
static const char UMBD[] = "#UMBD";
static const char SLEP[] = "#SLEP";

static void cmd_arm(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret, void * user UNUSED){
	//Check if all ports are on
	int status = rnhPortStatus();
	if(status != RNH_PORT_ALL){
		ret->data[0] = 'P';
		chsnprintf(ret->data+1, 2, "%x", status);
		ret->len = 3;
		return;
	}
	//Check if any ports are faulting
	int fault = rnhPortFault();
	if(fault != 0){
		ret->data[0] = 'F';
		chsnprintf(ret->data+1, 2, "%x", fault);
		ret->len = 3;
		return;
	}
	//Check if the battery has alarms
	int alarms[3] = {crntAlarms[0], crntAlarms[1], crntAlarms[2]};
	if(alarms[0] || alarms[1] || alarms[2]){
		ret->data[0] = 'A';
		chsnprintf(ret->data+1, 6, "%x%x%x", alarms[0], alarms[1], alarms[2]);
		ret->len=7;
		return;
	}

	//Check if shore power is on
	if(BQ24725_ACOK()){
		ret->data[0] = 'S';
		ret->len=1;
		return;
	}
	//Otherwise in arm state
	ret->len = 9;
	strncpy(ret->data, "go go go!", 9);
}

static void cmd_safe(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret UNUSED, void * user UNUSED){
	rrdySet(FALSE);
}

static void cmd_time(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret, void * user UNUSED){
	uint64_t time_ns = rtcGetTimeUnixUsec(&RTCD1) * 1000;
	chsnprintf(ret->data, 16, "%X%X", time_ns >> 32, time_ns);
	ret->len = 16;
}

static void cmd_rocketready(struct RCICmdData * cmd, struct RCIRetData * ret, void * user UNUSED){
	if(cmd->len == 1){
		rrdySet(cmd->data[0] == 'A');
	}
	if(rrdyStatus()){
		ret->data[0] = '1';
	} else {
		ret->data[0] = '0';
	}
	ret->len = 1;
}

static void cmd_sleep(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret, void * user UNUSED){
	if(rnhPortStatus()){
		ret->data[0] = 'P';
		ret->len = 1;
		return;
	}
	if(BQ24725_ACOK()){
		ret->data[0] = 'S';
		ret->len = 1;
		return;
	}
	KS8999_enable(FALSE);
}

static void cmd_umbdet(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret, void * user UNUSED){
	if(umbilical()){
		ret->data[0] = '1';
	} else {
		ret->data[0] = '0';
	}
	ret->len = 1;
}

/* Hardware handling callbacks	*/

static void umbdet_interrupt(EXTDriver * extp UNUSED, expchannel_t channel UNUSED){
	chSysLockFromIsr();
	chEvtBroadcastI(&UMBDET);
	chSysUnlockFromIsr();
}

static int umbstate;
static EvTimer umbdebounce;
static void umbdet_handler(eventid_t id UNUSED){
	umbstate = umbilical();
	umbdet_socket.buffer[0] = umbstate;
	seqWrite(&umbdet_socket, 1);
	evtStart(&umbdebounce);
}

static void umbdet_debounce(eventid_t id UNUSED){
	if(umbstate != umbilical()){
		umbdet_socket.buffer[0] = !umbstate;
		seqWrite(&umbdet_socket, 1);
	}
	evtStop(&umbdebounce);
}

static void ACOK_cb(EXTDriver *extp UNUSED, expchannel_t channel UNUSED) {
	if(BQ24725_ACOK()){
		ledOn(LED_ACOK);
		chSysLockFromIsr();
		chEvtBroadcastI(&ACOK);
		chSysUnlockFromIsr();
		KS8999_enable(TRUE);
	}else{
		ledOff(LED_ACOK);
	}
}

static void BQ24725_SetCharge(eventid_t id UNUSED){
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


static const struct swap BQ3060_swaps[] = {
	SWAP_FIELD(struct BQ3060Data, Temperature),
	SWAP_FIELD(struct BQ3060Data, TS1Temperature),
	SWAP_FIELD(struct BQ3060Data, TS2Temperature),
	SWAP_FIELD(struct BQ3060Data, TempRange),
	SWAP_FIELD(struct BQ3060Data, Voltage),
	SWAP_FIELD(struct BQ3060Data, Current),
	SWAP_FIELD(struct BQ3060Data, AverageCurrent),
	SWAP_FIELD(struct BQ3060Data, CellVoltage1),
	SWAP_FIELD(struct BQ3060Data, CellVoltage2),
	SWAP_FIELD(struct BQ3060Data, CellVoltage3),
	SWAP_FIELD(struct BQ3060Data, CellVoltage4),
	SWAP_FIELD(struct BQ3060Data, PackVoltage),
	SWAP_FIELD(struct BQ3060Data, AverageVoltage),
	{0},
};

static void BQ3060_SendData(eventid_t id UNUSED){
	struct BQ3060Data data;
	BQ3060_get_data(&data);
	write_swapped(BQ3060_swaps, &data, battery_socket.buffer);
	seqWrite(&battery_socket, len_swapped(BQ3060_swaps));
}

static const struct swap port_swaps[] = {
	SWAP_ARRAY(struct rnhPortCurrent, current),
	{0}
};

static void portCurrent_SendData(eventid_t id UNUSED){
	struct rnhPortCurrent sample;
	rnhPortGetCurrentData(&sample);
	sample.current[4] = BQ24725_IMON();
	write_swapped(port_swaps, &sample, port_socket.buffer);
	seqWrite(&port_socket, len_swapped(port_swaps));
}

static void batteryFault_Handler(eventid_t id UNUSED) {
	alarm_socket.buffer[0] = htons(crntAlarms[0]);
	alarm_socket.buffer[1] = htons(crntAlarms[1]);
	alarm_socket.buffer[2] = htons(crntAlarms[2]);
	seqWrite(&port_socket, sizeof(crntAlarms));
}

void main(void) {
	// Configuration
	static const struct led* leds[] = {
		&GREEN,
		&RED,
		NULL
	};
	static struct led_config led_cfg = {
		.cycle_ms = 500,
		.start_ms = 2500,
		.led = leds
	};
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
	struct RCICommand commands[] = {
		{TIME, cmd_time, NULL},
		{ARM, cmd_arm, NULL},
		{SAFE, cmd_safe, NULL},
		{RRDY, cmd_rocketready, NULL},
		{SLEP, cmd_sleep, NULL},
		{UMBD, cmd_umbdet, NULL},
		RCI_CMD_PORT,
		RCI_CMD_VERS,
		{NULL}
	};

	// Start Chibios
	halInit();
	chSysInit();
	// Start Diagnostics
	ledStart(&led_cfg);
#ifdef FLIGHT
	iwdgStart();
#endif
	//Init hardware
	BQ24725Start(&BQConf);
	BQ3060Start(&rnh3060conf);
	KS8999Start();
	rnhPortStart();
	// Start charging if possible
	if(BQ24725_ACOK()){
		ledOn(LED_ACOK);
		BQ24725_SetCharge(0);
	}

	//Init networking
	lwipThreadStart(RNH_LWIP);
	RCICreate(commands);

	// Set up sockets
	seqSocket(&battery_socket, RNH_BATTERY_ADDR);
	chDbgAssert(battery_socket.socket >=0, DBG_PREFIX"Battery socket failed", NULL);

	seqSocket(&port_socket, RNH_PORT_ADDR);
	chDbgAssert(port_socket.socket >=0, DBG_PREFIX"Port socket failed", NULL);

	seqSocket(&alarm_socket, RNH_ALARM_ADDR);
	chDbgAssert(alarm_socket.socket >=0, DBG_PREFIX"Alarm socket failed", NULL);

	seqSocket(&umbdet_socket, RNH_UMBDET_ADDR);
	chDbgAssert(umbdet_socket.socket >=0, DBG_PREFIX"Umbilical detect socket failed", NULL);

	connect(battery_socket.socket, FC_ADDR, sizeof(struct sockaddr));
	connect(port_socket.socket, FC_ADDR, sizeof(struct sockaddr));
	connect(alarm_socket.socket, FC_ADDR, sizeof(struct sockaddr));
	connect(umbdet_socket.socket, FC_ADDR, sizeof(struct sockaddr));
	evtInit(&umbdebounce, MS2ST(25));
	struct pin umbdetpin = {GPIOC, GPIO_C13_UMB_DETECT};
	extAddCallback(&umbdetpin, EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART, umbdet_interrupt);
	extUtilsStart();
	// Set up event system
	struct EventListener batt0, batt1, port0, batt_fault, umbdet, umbdeb;
	chEvtRegister(&ACOK, &batt0, 0);
	chEvtRegister(&BQ3060_data_ready, &batt1, 1);
	chEvtRegister(&rnhPortCurrent, &port0, 2);
	chEvtRegister(&BQ3060_battery_fault, &batt_fault, 3);
	chEvtRegister(&UMBDET, &umbdet, 4);
	chEvtRegister(&(umbdebounce.et_es), &umbdeb, 5);
	const evhandler_t evhndl[] = {
		BQ24725_SetCharge,
		BQ3060_SendData,
		portCurrent_SendData,
		batteryFault_Handler,
		umbdet_handler,
		umbdet_debounce
	};

	while (TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
