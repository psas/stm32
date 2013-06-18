#include "ch.h"
#include "hal.h"
#include "pwm_lld.h"


/*
JS DS8717 Servo
http://www.servodatabase.com/servo/jr/ds8717
Pulse widths range from 333 uS to 1520 uS
This implies that the period should be 1520 uS, or 1.526 mS

A value of 20000 = 1 sec.  20000 * 1.526*10^-3 = 30.52
*/



unsigned int pulseWidth = 150;
//unsigned int pulseWidth = 15;
unsigned int period = 333;
//unsigned int period = 67;

static PWMConfig pwmcfg = {
    6000000, /* 10Khz PWM clock frequency */
    19998, /* PWM period 1 second */
	 //67, //PWM period 1900 usec
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
