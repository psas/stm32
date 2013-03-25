#include "ch.h"
#include "hal.h"
#include "pwm_lld.h"

unsigned int pulseWidth = 5000;
unsigned int period = 20000;

static PWMConfig pwmcfg = {
    1000000, /* 10Khz PWM clock frequency */
    20000, /* PWM period 1 second */
    NULL,  /* No callback */
    /* Only channel 4 enabled */
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    },
    0
};

void pwmBegin() {
	palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
	pwmStart(&PWMD4, &pwmcfg);
	pwmEnableChannel(&PWMD4, 3, pulseWidth);
}

int getPulseWidth() {
	return pulseWidth;
}

int getPeriod() {
	return period;
}

void setPulseWidth(unsigned int width) {
	pwmDisableChannel(&PWMD4, 3);
	pwmEnableChannel(&PWMD4, 3, width);
	pulseWidth = width;
}

void setPeriod(unsigned int per)  {
	pwmChangePeriod(&PWMD4, per);
	period = per;
}
