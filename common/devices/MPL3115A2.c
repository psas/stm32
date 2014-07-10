/*! \file MPL3115A2.c
 *
 * Freescale Semiconductor Pressure sensor
 */

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_hal.h"

#include "MPL3115A2.h"

static int initialized;
EVENTSOURCE_DECL(MPL3115A2DataEvt);
static EVENTSOURCE_DECL(MPL3115A2Interrupt);

#define MPL3115A2_ADDR 0x60
static const systime_t I2C_TIMEOUT = MS2ST(400);

static I2CDriver * I2CD;
static struct MPL3115A2Data lastsample;


int MPL3115A2_Get(uint8_t register_id, uint8_t* data, int len){
    chDbgAssert(initialized, DBG_PREFIX"MPL315A2 driver not initialized", NULL);

    uint8_t tx[] = {register_id};
    i2cflags_t errors;
    i2cAcquireBus(I2CD);
    msg_t status = i2cMasterTransmitTimeout(I2CD,  MPL3115A2_ADDR, tx, sizeof(tx), data, len, I2C_TIMEOUT);
    switch(status){
    case RDY_OK:
        i2cReleaseBus(I2CD);
        break;
    case RDY_RESET:
        errors = i2cGetErrors(I2CD);
        i2cReleaseBus(I2CD);
        return errors;
    case RDY_TIMEOUT:
        i2cReleaseBus(I2CD);
        return RDY_TIMEOUT;
    default:
        i2cReleaseBus(I2CD);
    }

    return RDY_OK;
}

int MPL3115A2_Set(uint8_t register_id, uint8_t data){
    chDbgAssert(initialized, DBG_PREFIX"MPL315A2 driver not initialized", NULL);

    uint8_t tx[] = {register_id, data};
    i2cflags_t errors;
    i2cAcquireBus(I2CD);
    msg_t status = i2cMasterTransmitTimeout(I2CD, MPL3115A2_ADDR, tx, sizeof(tx), NULL, 0, I2C_TIMEOUT);
    switch(status){
    case RDY_OK:
        i2cReleaseBus(I2CD);
        break;
    case RDY_RESET:
        errors = i2cGetErrors(I2CD);
        i2cReleaseBus(I2CD);
        return errors;
    case RDY_TIMEOUT:
        i2cReleaseBus(I2CD);
        return RDY_TIMEOUT;
    default:
        i2cReleaseBus(I2CD);
    }

    return RDY_OK;
}


static void init(void){
    MPL3115A2_Set(MPL_PT_DATA_CFG, 0x7); // DREM | PDEFE | TDEFE
    MPL3115A2_Set(MPL_CTRL_REG1, 0x1);   // ACTIVE mode
    MPL3115A2_Set(MPL_CTRL_REG4, 0x80);  // DRDY interrupt
    MPL3115A2_Set(MPL_CTRL_REG5, 0x80);  // DRDY on int1
}

static void get_data(eventid_t id UNUSED){
    uint8_t buf[6];
    MPL3115A2_Get(MPL_STATUS, buf, sizeof(buf));
    chSysLock();
    lastsample.status = buf[0];
    lastsample.pressure = buf[1] << 16 | buf[2] << 8 | buf[3];
    lastsample.temperature = buf[4] << 8 | buf[5];
    chSysUnlock();
    chEvtBroadcast(&MPL3115A2DataEvt);
}


static WORKING_AREA(wa, 1024);
static msg_t commthd(void * p UNUSED){
    init();

    struct EventListener interrupt;
    static const evhandler_t evhndl[] = {
            get_data
    };
    chEvtRegister(&MPL3115A2Interrupt, &interrupt, 0);
    while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
    }
    return -1;
}

static void onInterrupt(EXTDriver *extp UNUSED, expchannel_t channel UNUSED){
    chSysLockFromIsr();
    chEvtBroadcastI(&MPL3115A2Interrupt);
    chSysUnlockFromIsr();
}

void MPL3115A2GetData(struct MPL3115A2Data * data){
    chSysLock();
    memcpy(data, &lastsample, sizeof(lastsample));
    chSysUnlock();
}

void MPL3115A2Start(struct MPL3115A2Config * conf) {
    I2CD = conf->i2cd;
    static const I2CConfig i2cfg = {
        OPMODE_I2C,
        400000,
        FAST_DUTY_CYCLE_2,
    };
    i2cUtilsStart(I2CD, &i2cfg, &(conf->pins));

    palSetPadMode(conf->interrupt.port, conf->interrupt.pad, PAL_MODE_INPUT | PAL_STM32_OSPEED_HIGHEST);
    extAddCallback(&(conf->interrupt), EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, onInterrupt);
    extUtilsStart();
    chThdCreateStatic(wa, sizeof(wa), NORMALPRIO, commthd, NULL);
    initialized=TRUE;
}
