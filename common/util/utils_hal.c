#include "hal.h"
#include "utils_hal.h"

#if DEBUG_MPU9150

/* see hal/include/i2c.h */
typedef struct i2c_error_info {
    const char* err_string;
    int         error_number;
} i2c_error_info;

const char* i2c_errno_str(int32_t err) ;

#endif

#if DEBUG_MPU9150 || defined(__DOXYGEN__)
    static i2c_error_info i2c_debug_errors[] = {
            {"I2C_NO_ERROR    ",   0x00},
            {"I2C_BUS_ERROR   ",   0x01},
            {"I2C_ARBIT_LOST  ",   0x02},
            {"I2C_ACK_FAIL    ",   0x04},
            {"I2CD_ACK_FAILURE",   0x04},
            {"I2CD_OVERRUN    ",   0x08},
            {"I2CD_PEC_ERROR  ",   0x10},
            {"I2CD_TIMEOUT    ",   0x20},
            {"I2CD_SMB_ALERT  ",   0x40}
    };

    const char* i2c_errno_str(int32_t err) {
        uint8_t count_errors = sizeof(i2c_debug_errors)/sizeof(i2c_error_info);
        uint8_t i            = 0;

        for(i=0; i<count_errors; ++i) {
            if(i2c_debug_errors[i].error_number == err) {
                return i2c_debug_errors[i].err_string;
            }
        }
        return "I2C Error Unknown";
    }
#endif


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
