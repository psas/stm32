#ifndef BMP180_H_
#define BMP180_H_

#include "utils_hal.h"
#include "evtimer.h"

struct BMP180Config {
	I2CDriver * i2cd;
	I2CPins pins;
};

struct BMP180Data {
	uint32_t pressure;
	uint8_t temperature;
};

extern EventSource BMP180DataEvt;
extern EvTimer BMP180Timer;

void BMP180_start(const struct BMP180Config * conf);
void BMP180_getSample(struct BMP180Data * data);
int BMP180_softReset(void);
int BMP180_id(uint8_t * id);
void BMP180_pump(eventid_t e);

#endif
