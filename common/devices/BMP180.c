#include "BMP180.h"
#include "utils_general.h"
#include "utils_hal.h"
#include "evtimer.h"
#include "ch.h"
#include "hal.h"
#include "string.h"

#define BMP180_ADDR 0x77

#define OUT_XLSB 0xF8
#define OUT_LSB 0xF7
#define OUT_MSB 0xF6
#define CTRL_MEAS 0xF4
#define SOFT 0xE0
#define ID 0xD0

#define TEMPERATURE 0x0E
#define PRESSURE 0x14

#define SCO 0x20

#define OSS1 0x00
#define OSS2 0x40
#define OSS4 0x80
#define OSS8 0xC0


EVENTSOURCE_DECL(BMP180DataEvt);
EvTimer BMP180Timer;

static int initialized;
static const systime_t I2C_TIMEOUT = MS2ST(400);

static I2CDriver * I2CD;
static const struct BMP180Config * CONF;
static struct BMP180Data lastsample;

static int handle_i2c_return(msg_t status) {
	i2cflags_t errors;
	switch(status){
	case RDY_OK:
		break;
	case RDY_RESET:
		errors = i2cGetErrors(I2CD);
		return errors;
	case RDY_TIMEOUT:
	/* On a timeout ChibiOS will set the bus into I2C_LOCKED, which will
	 * cause an assert to be hit if a bus transfer is tried again.
	 * I2C_LOCKED can only be cleared by i2cStart(), but i2cStart will hit
	 * an assert if i2cStop is not issued before restarting.
	 * This seems like a really terrible way to handle timeous*/
		i2cStop(I2CD);
		i2cStart(I2CD, I2CD->config);
		return RDY_TIMEOUT;
	}
	return RDY_OK;
}


static int set(int id, uint8_t data){
	chDbgAssert(initialized, "BMP180 not initialized (set)", NULL);
	uint8_t tx[2] = {id, data};
	i2cAcquireBus(I2CD);
	msg_t status = i2cMasterTransmitTimeout(I2CD, BMP180_ADDR, tx, sizeof(tx), NULL, 0, I2C_TIMEOUT);
	int r = handle_i2c_return(status);
	if(r) {
		i2cReleaseBus(I2CD);
		return r;
	}
	i2cReleaseBus(I2CD);
	return RDY_OK;
}


static int get(int id, void *data, unsigned int len){
	chDbgAssert(initialized, "BMP180 not initialized (get)", NULL);
	uint8_t tx[1] = {id};
	uint8_t rx[4] = {0};
	i2cAcquireBus(I2CD);
	msg_t status = i2cMasterTransmitTimeout(I2CD, BMP180_ADDR, tx, sizeof(tx), rx, sizeof(rx), I2C_TIMEOUT);
	int r = handle_i2c_return(status);
	if(r) {
		i2cReleaseBus(I2CD);
		return r;
	}
	i2cReleaseBus(I2CD);
	memcpy(data, rx, len);
	return RDY_OK;
}

void BMP180_getSample(struct BMP180Data *data){
	memcpy(data, &lastsample, sizeof(lastsample));
};

int BMP180_softReset(void) {
	return set(SOFT, 0xB6);
}

int BMP180_id(uint8_t * id) {
	return get(ID, id, 1);
}

void BMP180_pump(eventid_t e UNUSED) {
	static int i = 100;
	switch(i) {
	case 99:
		get(OUT_MSB, &lastsample.pressure, 3);
		set(CTRL_MEAS, TEMPERATURE | SCO | OSS1);
		break;
	case 100:
		get(OUT_MSB, &lastsample.temperature, 2);
		set(CTRL_MEAS, PRESSURE | SCO | OSS8);
		i = 1;
		break;
	default:
		get(OUT_MSB, &lastsample.pressure, 3);
		set(CTRL_MEAS, PRESSURE | SCO | OSS8);
	}
	++i;
	chEvtBroadcast(&BMP180DataEvt);
}

void BMP180_start(const struct BMP180Config * conf) {

	static const I2CConfig i2cfg = {
		OPMODE_I2C,
		400000,
		FAST_DUTY_CYCLE_2,
	};
	i2cUtilsStart(conf->i2cd, &i2cfg, &conf->pins);
	I2CD = conf->i2cd;
	CONF = conf;

	evtInit(&BMP180Timer, MS2ST(10));
	evtStart(&BMP180Timer);

	initialized=TRUE;
}
