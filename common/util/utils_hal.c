#include "hal.h"
#include "utils_general.h"
#include "utils_hal.h"

/*
 * EXT subsystem
 * ============= **************************************************************
 */

#if HAL_USE_EXT
static EXTConfig extconfig;
//TODO: Ideally should have an EXTDriver as an argument, but there's no good
//      way to dynamically have an EXTConfig for each one.
void extAddCallback(const struct pin * pin, uint32_t mode, extcallback_t cb){
    EXTDriver *EXTD = &EXTD1;

    //If an extconfig already exists, copy it over to our struct
    if(EXTD->config != NULL && EXTD->config != &extconfig){
        int i;
        for(i = 0; i < EXT_MAX_CHANNELS; ++i){
            extconfig.channels[i].cb = EXTD->config->channels[i].cb;
            extconfig.channels[i].mode = EXTD->config->channels[i].mode;
        }
    }

    uint32_t CHANNEL_MODE = 0;
    if(pin->port == GPIOA){
        CHANNEL_MODE = EXT_MODE_GPIOA;
    } else if(pin->port == GPIOB){
        CHANNEL_MODE = EXT_MODE_GPIOB;
    } else if(pin->port == GPIOC){
        CHANNEL_MODE = EXT_MODE_GPIOC;
    } else if(pin->port == GPIOD){
        CHANNEL_MODE = EXT_MODE_GPIOD;
    } else if(pin->port == GPIOE){
        CHANNEL_MODE = EXT_MODE_GPIOE;
    } else if(pin->port == GPIOF){
        CHANNEL_MODE = EXT_MODE_GPIOF;
    } else if(pin->port == GPIOG){
        CHANNEL_MODE = EXT_MODE_GPIOG;
    } else if(pin->port == GPIOH){
        CHANNEL_MODE = EXT_MODE_GPIOH;
    } else if(pin->port == GPIOI){
        CHANNEL_MODE = EXT_MODE_GPIOI;
    }

    //TODO: warn if overwriting?
    extconfig.channels[pin->pad].cb = cb;
    extconfig.channels[pin->pad].mode = (mode & ~EXT_MODE_GPIO_MASK) | CHANNEL_MODE;


    if(EXTD->state == EXT_ACTIVE && (mode & EXT_CH_MODE_AUTOSTART)){
        extChannelEnable(EXTD, pin->pad);
    }
}

//fixme: This is kind of a cheesey way of doing this, but doing it better
//       will probably only happen when extAddCallback's todo is fixed.
void extUtilsStart(void){
    extStart(&EXTD1, &extconfig);
}
#endif /* HAL_USE_EXT */


/*
 * I2C subsystem
 * ============= **************************************************************
 */
#if 0 //HAL_USE_I2C
// TODO: SMBA?
// TODO: Can the pins alt function table be used in some way?

#define ALT_FUNC_I2C 4 // See STM32F407 Datasheet, Table 9
#define PINMODE PAL_MODE_ALTERNATE(ALT_FUNC_I2C)\
              | PAL_STM32_PUDR_PULLUP\
              | PAL_STM32_OSPEED_HIGHEST\
              | PAL_STM32_OTYPE_OPENDRAIN;

void utils_i2cStart(I2CDriver * driver, I2CConfig * config, I2CPins * pins){
    chDbgCheck(driver && config && pins, utils_i2cStart);
    chDbgAssert(driver->state != I2C_UNINIT,
                DBG_PREFIX "I2C hal driver must be enabled",
                NULL);

    palSetPadMode(pins->SDA.port, pins->SDA.pad, PINMODE);
    palSetPadMode(pins->SCL.port, pins->SCL.pad, PINMODE);

    if(driver->state == I2C_STOP){
        i2cStart(driver, &config);
    }else{
        //Verify I2C configs match.
        chDbgAssert(driver->config->opmode == config->opmode,
                DBG_PREFIX "requested opmode does not match previously configured",
                NULL);
        chDbgAssert(driver->config->clock_speed == config->clock_speed,
                DBG_PREFIX "requested clock speed does not match previously configured",
                NULL);
        chDbgAssert(driver->config->duty_cycle == config->duty_cycle,
                DBG_PREFIX "requested duty cycle does not match previously configured",
                NULL);
    }
}

int SMBus_Get(uint8_t register_id, uint16_t* data){
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

int SMBus_Set(uint8_t register_id, uint16_t data){
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

void chprint_i2c_state(BaseSequentialStream * chp, i2cstate_t state){
    switch(state){
    case_chprint(chp, I2C_UNINIT)
    case_chprint(chp, I2C_STOP)
    case_chprint(chp, I2C_READY)
    case_chprint(chp, I2C_ACTIVE_TX)
    case_chprint(chp, I2C_ACTIVE_RX)
    case_chprint(chp, I2C_LOCKED)
    default:
        chprintf(chp, "unknown");
    }
}

void chprint_i2c_error(BaseSequentialStream * chp, int err){
    switch(err){
    case_chprint(chp, I2CD_NO_ERROR)
    case_chprint(chp, I2CD_BUS_ERROR)
    case_chprint(chp, I2CD_ARBITRATION_LOST)
    case_chprint(chp, I2CD_ACK_FAILURE)
    case_chprint(chp, I2CD_OVERRUN)
    case_chprint(chp, I2CD_PEC_ERROR)
    case_chprint(chp, I2CD_TIMEOUT)
    case_chprint(chp, I2CD_SMB_ALERT)
    default:
        chprintf(chp, "unknown");
    }
}
#endif /* HAL_USE_I2C*/

