/*
 */
#include <stddef.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "utils_general.h"

#include "RNH.h"

#define NUM_PORT 8

EVENTSOURCE_DECL(portCurrent_drdy);

static uint16_t outBuffer[8];
static int activeport;

static void ADCCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n UNUSED){
    static uint8_t done_samples = 0; // bitfield of completed samples
    int i = 0;
    if(adcp == &ADCD1){
        i = activeport*2;
    }else if(adcp == &ADCD2){
        i = activeport*2 + 1;
    }

    if(adcp == &ADCD1 || adcp == &ADCD2){
        outBuffer[i] = buffer[0];
        done_samples |= 1 << i;
    }

    if(done_samples == 0xFF){
        done_samples = 0;
        chSysLockFromIsr();
        chEvtBroadcastI(&portCurrent_drdy);
        chSysUnlockFromIsr();
    }

}

static void select_port_imon(int port){
    switch(port){
    case 0:
        palClearPad(GPIOD, GPIO_D7_IMON_A0);
        palClearPad(GPIOD, GPIO_D8_IMON_A1);
        break;
    case 1:
        palSetPad(GPIOD, GPIO_D7_IMON_A0);
        palClearPad(GPIOD, GPIO_D8_IMON_A1);
        break;
    case 2:
        palClearPad(GPIOD, GPIO_D7_IMON_A0);
        palSetPad(GPIOD, GPIO_D8_IMON_A1);
        break;
    case 3:
        palSetPad(GPIOD, GPIO_D7_IMON_A0);
        palSetPad(GPIOD, GPIO_D8_IMON_A1);
        break;
    }
}

static void StartADCSample(GPTDriver *gptp UNUSED){
    static adcsample_t buffer0 = 0;
    static adcsample_t buffer1 = 0;
    static ADCConversionGroup bank0 = {
            .circular = FALSE,
            .num_channels = 1,
            .end_cb = ADCCallback,
            .error_cb = NULL,
            .cr1 = ADC_CR1_EOCIE,
            .cr2 = ADC_CR2_SWSTART,
            .smpr1 = ADC_SMPR1_SMP_AN10(ADC_SAMPLE_480),
            .smpr2 = 0,
            .sqr1 = ADC_SQR1_NUM_CH(1),
            .sqr2 = 0,
            .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10)
    };

    static ADCConversionGroup bank1 = {
            .circular = FALSE,
            .num_channels = 1,
            .end_cb = ADCCallback,
            .error_cb = NULL,
            .cr1 = ADC_CR1_EOCIE,
            .cr2 = ADC_CR2_SWSTART,
            .smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_480),
            .smpr2 = 0,
            .sqr1 = ADC_SQR1_NUM_CH(1),
            .sqr2 = 0,
            .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
    };

    select_port_imon(activeport);
    chSysLockFromIsr();
    adcStartConversionI(&ADCD1, &bank0, &buffer0, 1);
    adcStartConversionI(&ADCD2, &bank1, &buffer1, 1);
    chSysUnlockFromIsr();
    activeport = (activeport + 1) % 4;
}

void portCurrentGetData(uint16_t * data){
//    chSysLock();
    memcpy(data, &outBuffer, sizeof(outBuffer));
//    chSysUnlock();
}

void RNH_init(void){
    static ADCConfig conf = {0}; //nothing to config on STM32

    adcStart(&ADCD1, &conf);
    adcStart(&ADCD2, &conf);

    static GPTConfig gptcfg = {
            .frequency = 40000,
            .callback = StartADCSample,
            .dier = 0,
    };
    gptStart(&GPTD2, &gptcfg);
    gptStartContinuous(&GPTD2, 1000);

}

static uint32_t power[NUM_PORT] =
    {GPIO_E0_NODE1_N_EN, GPIO_E1_NODE2_N_EN,
     GPIO_E2_NODE3_N_EN, GPIO_E3_NODE4_N_EN, GPIO_E4_NC,
     GPIO_E5_NODE6_N_EN, GPIO_E6_NODE7_N_EN, GPIO_E7_NODE8_N_EN};

//todo: interrupt on faults?

static uint32_t fault[NUM_PORT] =
    {GPIO_E8_NODE1_N_FLT, GPIO_E9_NODE2_N_FLT,
     GPIO_E10_NODE3_N_FLT, GPIO_E11_NODE4_N_FLT, GPIO_E12_NC,
     GPIO_E13_NODE6_N_FLT, GPIO_E14_NODE7_N_FLT, GPIO_E15_NODE8_N_FLT};

RNH_port RNH_power(RNH_port port, RNH_action action){

    int active_port[NUM_PORT] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    if(port & RNH_PORT_1){
        active_port[0] = TRUE;
    }
    if(port & RNH_PORT_2){
        active_port[1] = TRUE;
    }
    if(port & RNH_PORT_3){
        active_port[2] = TRUE;
    }
    if(port & RNH_PORT_4){
        active_port[3] = TRUE;
    }
    if(port & RNH_PORT_6){
        active_port[5] = TRUE;
    }
    if(port & RNH_PORT_7){
        active_port[6] = TRUE;
    }

    RNH_port return_port = 0;
    int i;
    for(i = 0; i < NUM_PORT; ++i){
        if(active_port[i]){
            switch(action){
            case RNH_PORT_ON:
                palClearPad(GPIOE, power[i]);
                if(palReadPad(GPIOE, power[i])){
                    return_port |= 1<<i;
                }
                break;
            case RNH_PORT_OFF:
                palSetPad(GPIOE, power[i]);
                if(palReadPad(GPIOE, power[i])){
                    return_port |= 1<<i;
                }
                break;
            case RNH_PORT_FAULT:
                if(palReadPad(GPIOE, fault[i])){
                    return_port |= 1<<i;
                }
                break;
            case RNH_PORT_STATUS:
            default:
                if(palReadPad(GPIOE, power[i])){
                    return_port |= 1<<i;
                }
                break;
            }
        }
    }
    return return_port;
}


