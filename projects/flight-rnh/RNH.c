/*
 */

#include "ch.h"
#include "hal.h"
#include "RNH.h"

#define NUM_PORT 8

static uint32_t power[NUM_PORT] =
    {GPIO_E4_NC, GPIO_E0_NODE1_N_EN, GPIO_E1_NODE2_N_EN,
     GPIO_E2_NODE3_N_EN, GPIO_E3_NODE4_N_EN, GPIO_E4_NC,
     GPIO_E5_NODE6_N_EN, GPIO_E6_NODE7_N_EN};

static uint32_t fault[NUM_PORT] =
    {GPIO_E12_NC, GPIO_E8_NODE1_N_FLT, GPIO_E9_NODE2_N_FLT,
     GPIO_E10_NODE3_N_FLT, GPIO_E11_NODE4_N_FLT, GPIO_E12_NC,
     GPIO_E13_NODE6_N_FLT, GPIO_E14_NODE7_N_FLT};

RNH_port RNH_power(RNH_port port, RNH_action action){

    int active_port[NUM_PORT] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    if(port & RNH_PORT_1){
        active_port[1] = TRUE;
    }
    if(port & RNH_PORT_2){
        active_port[2] = TRUE;
    }
    if(port & RNH_PORT_3){
        active_port[3] = TRUE;
    }
    if(port & RNH_PORT_4){
        active_port[4] = TRUE;
    }
    if(port & RNH_PORT_6){
        active_port[6] = TRUE;
    }
    if(port & RNH_PORT_7){
        active_port[7] = TRUE;
    }

    RNH_port return_port = 0;
    int i;
    for(i = 0; i < NUM_PORT; ++i){
        if(active_port[i]){
            switch(action){
            case RNH_PORT_ON:
                palClearPad(GPIOE, power[i]);
                break;
            case RNH_PORT_OFF:
                palSetPad(GPIOE, power[i]);
                break;
            case RNH_PORT_FAULT:
                if(palReadPad(GPIOE, fault[i])){
                    return_port |= 1<<i; //FIXME
                }
                break;
            case RNH_PORT_STATUS:
            default:
                if(palReadPad(GPIOE, power[i])){
                    return_port |= 1<<i; //FIXME
                }
            }

        }
    }

    return return_port;
}
