#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "utils_general.h"
#include "utils_hal.h"

#include "RNHPort.h"

typedef enum {
	RNH_PORT_STATUS = 0,
	RNH_PORT_FAULT = 1,
	RNH_PORT_ON = 2,
	RNH_PORT_OFF = 3,
	RNH_PORT_CURRENT_FREQ = 4
} RNHAction;

static void cmd_port(struct RCICmdData * cmd, struct RCIRetData * ret, void * user UNUSED);
const struct RCICommand RCI_CMD_PORT = {
	.name = "#PORT",
	.function = cmd_port,
	.user = NULL
};

#define NUM_PORT 8
static const uint32_t power[NUM_PORT] = {
	GPIO_E0_NODE1_N_EN,
	GPIO_E1_NODE2_N_EN,
	GPIO_E2_NODE3_N_EN,
	GPIO_E3_NODE4_N_EN,
	GPIO_E4_NC,
	GPIO_E5_NODE6_N_EN,
	GPIO_E6_NODE7_N_EN,
	GPIO_E7_NODE8_N_EN
};
static const uint32_t fault[NUM_PORT] = {
	GPIO_E8_NODE1_N_FLT,
	GPIO_E9_NODE2_N_FLT,
	GPIO_E10_NODE3_N_FLT,
	GPIO_E11_NODE4_N_FLT,
	GPIO_E12_NC,
	GPIO_E13_NODE6_N_FLT,
	GPIO_E14_NODE7_N_FLT,
	GPIO_E15_NODE8_N_FLT
};

#define portGPTfreq 80000

EVENTSOURCE_DECL(rnhPortCurrent);
static struct rnhPortCurrent outBuffer;

static void select_port_imon(int port){
	(port & 1) ? palSetPad(GPIOD, GPIO_D7_IMON_A0) : palClearPad(GPIOD, GPIO_D7_IMON_A0);
	(port & 2) ? palSetPad(GPIOD, GPIO_D8_IMON_A1) : palClearPad(GPIOD, GPIO_D8_IMON_A1);
}

#define SAMPLE_PORTS (RNH_PORT_ALL)

static void ADCCallback(ADCDriver *adcp UNUSED, adcsample_t *buffer, size_t n UNUSED){
	static uint8_t remaining_samples = SAMPLE_PORTS;

	remaining_samples &= ~(1 << (buffer - outBuffer.current));

	if(!remaining_samples){
		remaining_samples = SAMPLE_PORTS;
		chSysLockFromIsr();
		chEvtBroadcastI(&rnhPortCurrent);
		chSysUnlockFromIsr();
	}
	//FIXME: Ideally select_port_imon would go here, but coordinating between the two adcs is trickey
}

#define makeBankConversionGroup(channel) \
{ \
	.circular = FALSE, \
	.num_channels = 1, \
	.end_cb = ADCCallback, \
	.error_cb = NULL, \
	.cr1 = ADC_CR1_EOCIE, \
	.cr2 = ADC_CR2_SWSTART, \
	.smpr1 = ADC_SMPR1_SMP_AN ## channel (ADC_SAMPLE_480), \
	.smpr2 = 0, \
	.sqr1 = ADC_SQR1_NUM_CH(1), \
	.sqr2 = 0, \
	.sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN ## channel) \
}

static void StartADCSample(GPTDriver *gptp UNUSED){
	static int activeport = 0;
	static int select_port = 1;
	if(select_port){
		select_port_imon(activeport);
		select_port = 0;
		return;
	}
	select_port = 1;

	static ADCConversionGroup bank0 = makeBankConversionGroup(10);
	static ADCConversionGroup bank1 = makeBankConversionGroup(11);

	chSysLockFromIsr();
	int bank1port = activeport;
	int bank2port = activeport + 4;

	if((1 << bank1port) & SAMPLE_PORTS ){
		adcStartConversionI(&ADCD1, &bank0, &outBuffer.current[bank1port], 1);
	} else {
		outBuffer.current[bank1port] = 0;
	}
	if((1 << bank2port) & SAMPLE_PORTS){
		adcStartConversionI(&ADCD2, &bank1, &outBuffer.current[bank2port], 1);
	} else {
		outBuffer.current[bank2port] = 0;
	}
	chSysUnlockFromIsr();
	activeport = (activeport + 1) % 4;
}


static rnhPortFaultHandler fault_handler = NULL;
static void * fault_handler_data = NULL;

static void portErrorCallback (EXTDriver *extp UNUSED, expchannel_t channel){
	if(fault_handler){
		fault_handler(channel - NUM_PORT, fault_handler_data);
	}
}

void rnhPortStart(void){
	static ADCConfig conf = {0}; //nothing to config on STM32

	adcStart(&ADCD1, &conf);
	adcStart(&ADCD2, &conf);

	static GPTConfig gptcfg = {
		.frequency = portGPTfreq,
		.callback = StartADCSample,
		.dier = 0,
	};
	gptStart(&GPTD2, &gptcfg);
	rnhPortSetCurrentDataRate(RNH_PORT_CURRENT_DEFAULT_SAMPLE_RATE);


	for(int i = 0; i < NUM_PORT; ++i){
		extAddCallback( &(struct pin){.port=GPIOE, .pad=fault[i]}
		              , EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART
		              , portErrorCallback
		              );
	}
	extUtilsStart();
}

RNHPort rnhPortStatus(void){
	RNHPort return_port = 0;

	for(int i = 0; i < NUM_PORT; ++i){
		return_port |= palReadPad(GPIOE, power[i])<<i;
	}
	return (~return_port) & RNH_PORT_ALL;
};

RNHPort rnhPortFault(void){
	RNHPort return_port = 0;

	for(int i = 0; i < NUM_PORT; ++i){
		return_port |= palReadPad(GPIOE, fault[i])<<i;
	}
	return (~return_port) & RNH_PORT_ALL;
}

void rnhPortOn(RNHPort port){
	port &= RNH_PORT_ALL;

	for(int i = 0; i < NUM_PORT; ++i){
		if(port & 1<<i){
			palClearPad(GPIOE, power[i]);
		}
	}
}

void rnhPortOff(RNHPort port){
	port &= RNH_PORT_ALL;

	for(int i = 0; i < NUM_PORT; ++i){
		if(port & 1<<i){
			palSetPad(GPIOE, power[i]);
		}
	}
}

void rnhPortSetFaultHandler(rnhPortFaultHandler handler, void * data){
	chSysLock();
	fault_handler = handler;
	fault_handler_data = data;
	chSysUnlock();
}

void rnhPortGetCurrentData(struct rnhPortCurrent * measurement){
	chSysLock();
	*measurement = outBuffer;
	chSysUnlock();
 }

void rnhPortSetCurrentDataRate(unsigned freq){
	chDbgAssert(freq <= RNH_PORT_CURRENT_MAX_SAMPLE_RATE, "Setting rhnport sample rate too high", NULL);
	gptStopTimer(&GPTD2);
	gptStartContinuous(&GPTD2, portGPTfreq / 8 / freq);
}

static void cmd_port(struct RCICmdData * cmd, struct RCIRetData * ret, void * user UNUSED){
	if(cmd->len < 1){
		return;
	}

	RNHAction action = 0;
	if(cmd->data[0] == 'S'){ action = RNH_PORT_STATUS;
	} else if(cmd->data[0] == 'F'){ action = RNH_PORT_FAULT;
	} else if(cmd->data[0] == 'O'){ action = RNH_PORT_ON;
	} else if(cmd->data[0] == 'X'){ action = RNH_PORT_OFF;
	} else if(cmd->data[0] == 'Q'){ action = RNH_PORT_CURRENT_FREQ;
	}
	int data = 0;
	char tmp[3];
	tmp[2] = '\0';
	for(int i = 1; i < cmd->len; i+=2){
		data <<= 8;
		tmp[0] = cmd->data[i];
		tmp[1] = cmd->data[i+1];
		data |= (uint8_t) strtol(tmp, NULL, 16);
	}

	RNHPort status = 0;
	switch(action){
	case RNH_PORT_STATUS:
		status = rnhPortStatus();
		break;
	case RNH_PORT_FAULT:
		status = rnhPortFault();
		break;
	case RNH_PORT_ON:
		rnhPortOn(data);
		status = rnhPortStatus();
		break;
	case RNH_PORT_OFF:
		rnhPortOff(data);
		status = rnhPortStatus();
		break;
	case RNH_PORT_CURRENT_FREQ:
		if(data > RNH_PORT_CURRENT_MAX_SAMPLE_RATE){
			status = -1;
		}else{
			rnhPortSetCurrentDataRate(data);
		}
		return;
	default:
		return;
	}

	chsnprintf(ret->data, 2, "%x", status);
	ret->len = 2;
}

