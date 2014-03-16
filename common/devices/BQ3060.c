#include <stddef.h>
#include <stdbool.h>

#include "hal.h"
#include "BQ3060.h"


struct BQ3060Config rnh3060conf = {
    .I2CD = &I2CD1
};

static struct BQ3060Config * CONF;
static bool initialized = false;
static I2CDriver *I2CD;
static const systime_t I2C_TIMEOUT = MS2ST(400);

void BQ3060_init(struct BQ3060Config * conf){
    if(!conf){
        return;
    }
    CONF = conf;
    I2CD = CONF->I2CD;

    //todo: set I2C pins
    static const I2CConfig i2cfg = {
        OPMODE_SMBUS_HOST,
        100000,
        STD_DUTY_CYCLE,
    };
    if(I2CD->state == I2C_STOP){
        i2cStart(I2CD, &i2cfg);
    }

    initialized = true;
}

int BQ3060_Get(uint8_t register_id, uint16_t* data){
    if(initialized == false)
        return -1;

    uint8_t tx[1] = {register_id};
    uint8_t rx[2];
    i2cflags_t errors;
    i2cAcquireBus(I2CD);
    msg_t status = i2cMasterTransmitTimeout(I2CD, BQ3060_ADDR, tx, sizeof(tx), rx, sizeof(rx), I2C_TIMEOUT);
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

    *data = DATA_FROM_BYTES(rx[0], rx[1]);
    return RDY_OK;
}

int BQ3060_Set(uint8_t register_id, uint16_t data){
    if(initialized == false)
        return -1;

    uint8_t tx[3] = {register_id, LOWDATA_BYTE(data), HIGHDATA_BYTE(data)};
    i2cflags_t errors;
    i2cAcquireBus(I2CD);
    msg_t status = i2cMasterTransmitTimeout(I2CD, BQ3060_ADDR, tx, sizeof(tx), NULL, 0, I2C_TIMEOUT);
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


/* Danger:
 *
 * Safety status : 0x51
 * - expects 0s
 *
 * Safety Alert : 0x50
 * - expects 0s
 *
 * PFAlert : 0x52
 * - expects 0s
 *
 * PFStatus : 0x53
 * - expects 0s
 *
 *
 * Info:
 *
 * OperationStatus : 0x54
 * - PRES (MSB)
 * - XDSG (bit 5)
 * - XDSGI (bit 4)
 * - EDV2 (bit 3)
 *
 * ChargingStaus : 0x55
 * - XCHG (MSB)
 * - CHGSUSP (14)
 * - PCHG (13)
 * - LTCHG
 * - ST1CHG
 * - ST2CHG
 * - HTCHG
 * - CB
 * - everything else 0
 *
 *  PackVoltage : 0x5a
 *  - 16bit mv
 *
 *  AverageVoltage : 0x5d
 *  - 16b mv
 *
 *  TS1Temperatrue : 0x5e
 *  TS2Temperatrue : 0x5f
 *  BatteryStatus : 0x16
 *  Temperature : 0x08
 *  Voltage : 0x09
 *  Current : 0x0a
 *  AverageCurrent : 0x0b
 *  RemaingingCapacity : 0x0f
 *  - mah
 *  FullChagreCapacity : 0x10
 *  - mah
 *  ChargingCurrent : 0x14
 *  ChargingVoltage : 0x15
 *  BatteryStatus : 0x16
 *  CycleCount : 0x17
 *  CellVoltage4...1 : 0x3c...0x3f
 *
 *
 *
 *
 *

 */
