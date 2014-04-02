#ifndef UTILS_HAL_H_
#define UTILS_HAL_H_

#include "hal.h"

struct pin {
    ioportid_t port;
    uint16_t pad;
};

#if HAL_USE_EXT
void extUtilsStart(void);
void extAddCallback(const struct pin * pin, uint32_t mode, extcallback_t cb);
#endif

#if HAL_USE_I2C
typedef struct{
    struct pin SDA;
    struct pin SCL;
} I2CPins;

void utils_i2cStart(I2CDriver * driver, I2CConfig * config, I2CPins * pins);
void chprint_i2c_state(BaseSequentialStream * chp, i2cstate_t state);
void chprint_i2c_error(BaseSequentialStream * chp, int err);
#endif


#endif /* UTILS_HAL_H_ */
