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

void i2cUtilsStart(I2CDriver * driver, const I2CConfig * config, const I2CPins * pins);
int SMBusGet(I2CDriver * driver, uint8_t addr, uint8_t command, uint16_t* data);
int SMBusSet(I2CDriver * driver, uint8_t addr, uint8_t command, uint16_t data);
void chprintI2cState(BaseSequentialStream * chp, i2cstate_t state);
void chprintI2cError(BaseSequentialStream * chp, int err);
#endif


#endif /* UTILS_HAL_H_ */
