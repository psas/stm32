#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "evtimer.h"

#include "BQ3060.h"

#define UNUSED __attribute__((unused))

static struct BQ3060Config * CONF;
static bool initialized = false;
static I2CDriver *I2CD;
static const systime_t I2C_TIMEOUT = MS2ST(400);

EventSource BQ3060_data_ready;
EventSource BQ3060_battery_fault;

static struct BQ3060Data buffer;

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

/*void AtRate(int16_t rate){
    BQ3060_AtRate,
    BQ3060_AtRateTimeToFull,
    BQ3060_AtRateTimeToEmpty,
    BQ3060_AtRateOK
}
*/
/*
void StateOfCharge(){
    BQ3060_MaxError = 0x0c,
    BQ3060_RelativeStateOfCharge = 0x0d,
    BQ3060_AbsoluteStateOfCharge = 0x0e,
    BQ3060_RemainingCapacity = 0x0f,
    BQ3060_FullChargeCapacity = 0x10,
    BQ3060_RunTimeToEmpty = 0x11,
    BQ3060_AverageTimeToEmpty = 0x12,
    BQ3060_AverageTimeToFull = 0x13,
}
*/
/*
void DesiredCharge(){
    BQ3060_ChargingCurrent = 0x14,
    BQ3060_ChargingVoltage = 0x15,

}
*/
/*
void Safety(){
    BQ3060_SafetyAlert = 0x50,
    BQ3060_SafetyStatus = 0x51,
    BQ3060_PFAlert = 0x52,
    BQ3060_PFStatus = 0x53,
}
 */
/*
void Status(){
    BQ3060_BatteryStatus = 0x16,
    BQ3060_CycleCount = 0x17,
    BQ3060_StateOfHealth = 0x4f,
    BQ3060_OpertationStatus = 0x54,
    BQ3060_ChargingStatus = 0x55,
    BQ3060_FETStatus = 0x56,
}
 */
static void Physical(struct BQ3060Data * data){
    BQ3060_Get(BQ3060_Temperature, &(data->Temperature));
    BQ3060_Get(BQ3060_TS1Temperature, (uint16_t *)&(data->TS1Temperature));
    BQ3060_Get(BQ3060_TS2Temperature, (uint16_t *)&(data->TS2Temperature));
    BQ3060_Get(BQ3060_TempRange, &(data->TempRange));
    BQ3060_Get(BQ3060_Current, (uint16_t *)&(data->Current));
    BQ3060_Get(BQ3060_AverageCurrent, (uint16_t *)&(data->AverageCurrent));
    BQ3060_Get(BQ3060_Voltage, &(data->Voltage));
    BQ3060_Get(BQ3060_CellVoltage4, &(data->CellVoltage4));
    BQ3060_Get(BQ3060_CellVoltage3, &(data->CellVoltage3));
    BQ3060_Get(BQ3060_CellVoltage2, &(data->CellVoltage2));
    BQ3060_Get(BQ3060_CellVoltage1, &(data->CellVoltage1));
    BQ3060_Get(BQ3060_PackVoltage, &(data->PackVoltage));
    BQ3060_Get(BQ3060_AverageVoltage, &(data->AverageVoltage));
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

void BQ3060_get_data(struct BQ3060Data * data){
//    chSysLock();
    memcpy(data, &buffer, sizeof(buffer));
//    chSysUnlock();
}

static void read_handler(eventid_t id UNUSED){
    Physical(&buffer);
    chEvtBroadcast(&BQ3060_data_ready);
}

static WORKING_AREA(wa_read, 256);
static msg_t read_thread(void * p UNUSED){
    chRegSetThreadName("BQ3060");

    EvTimer timer;
    evtInit(&timer, S2ST(1));

    struct EventListener eltimer;
    uint16_t safetyData = 0;
    static const evhandler_t evhndl[] = {
            read_handler
    };
    chEvtRegister(&timer.et_es, &eltimer, 0);

    evtStart(&timer);
    while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	//if any battery issues have occurred we fire 
	//the event associated with BQ3060_battery_fault
	//read the safety alert register (0x50)
	BQ3060_Get(0x50,&safetyData);
	if (safetyData != 0) {
		chEvtBroadcast(&BQ3060_battery_fault);
	}
	//read the pending failure alert register (0x52)
	BQ3060_Get(0x52,&safetyData);
	if (safetyData != 0) {
		chEvtBroadcast(&BQ3060_battery_fault);
	}
	//read the permanent failure status register (0x53)
	BQ3060_Get(0x53,&safetyData);
	if (safetyData != 0) {
		chEvtBroadcast(&BQ3060_battery_fault);
	}
	chThdSleepMilliseconds(1000); 
    }

    return -1;
}

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

    chEvtInit(&BQ3060_data_ready);
    chThdCreateStatic(wa_read, sizeof(wa_read), NORMALPRIO, read_thread, NULL);
    initialized = true;
}
