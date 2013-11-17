#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbdetail.h"
#include "shell.h"
/* Simple PWM example */

//TODO: shell function to set period et al.

void cmd_pwm(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    (void)argc;
    uint32_t psc;

//    pwm_start();
//    chThdSleepMilliseconds(200);

    chprintf(chp, "pulse width:\t%d\r\n", PWMD4.tim->CCR[2]);
    chprintf(chp, "pwm state:\t%d\r\n", PWMD4.state);
    chprintf(chp, "pwm clock:\t%d\r\n", PWMD4.clock);
    chprintf(chp, "pwm freq:\t%d\r\n", PWMD4.config->frequency);
    chprintf(chp, "psc for freq 8000000:\t%d\r\n", (PWMD4.clock/8000000)-1);
    chprintf(chp, "psc:\t0x%x\t%d\r\n", (PWMD4.clock / PWMD4.config->frequency)-1,( PWMD4.clock / PWMD4.config->frequency)-1);
    psc = (PWMD4.clock / PWMD4.config->frequency) - 1;
    chprintf(chp, "(psc + 1) * pwmp->config->frequency:\t0x%x\t%d\r\n", (psc + 1) * PWMD4.config->frequency,(psc + 1) * PWMD4.config->frequency );

    chprintf(chp, "STM32_TIMCLK1\t%d\r\n", STM32_TIMCLK1);
    chprintf(chp, "STM32_PCLK1\t%d\r\n", STM32_PCLK1);
    chprintf(chp, "STM32_TIMCLK2\t%d\r\n", STM32_TIMCLK2);
    chprintf(chp, "STM32_PCLK2\t%d\r\n", STM32_PCLK2);

}

/*
 * STM32F40x includes 4 general purpose timers: TIM2, TIM3, TIM4, TIM5
 * TIM3 and TIM4 are 16 bit timers with PWM support.
 *
 * PWMD4 is a HAL defined variable of type PWMDriver - it is associated with
 * TIM4.
 */

#define  INIT_PWM_FREQ        1000000 /* 1 Mhz PWM clock frequency, 1 uS per tick */
#define  INIT_PWM_PERIOD      400     /* PWM period in freq ticks, 400uS */
#define  INIT_PWM_WIDTH_TICS  20

PWMConfig pwmcfg_led = {
        INIT_PWM_FREQ,      /* Frequency */
        INIT_PWM_PERIOD,    /* Period */
        NULL,               /* Callback (Not used here) */
        /* Only channel 4 enabled */
        {
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        },
        /* STM32 Specific config options */
        0, /* TIM_CR2  */
        0  /* TIM_DIER */
};

/*
 * PWM working thread
 */
static WORKING_AREA(waThread2, 128);
static msg_t pwm_thread(void *arg) {
	unsigned int pwm_period_new = INIT_PWM_PERIOD;
	bool         upcount        = TRUE;
	(void)arg;
	chRegSetThreadName("pwm_experiment");
	while (TRUE) {
		if(upcount) {
			pwm_period_new += 1;
		} else {
			pwm_period_new -= 1;
		}

		if(pwm_period_new > (INIT_PWM_PERIOD * 0.5)) {
			upcount = FALSE;
		}
		if(pwm_period_new < (INIT_PWM_WIDTH_TICS+1)) {
			upcount = TRUE;
		}
		pwmChangePeriod(&PWMD4, pwm_period_new);
		chThdSleepMilliseconds(20);
	}
	return -1;
}

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t led_thread(void *arg) {

	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE) {
		palTogglePad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(500);
	}
	return -1;
}

/*
 * Application entry point.
 */
void main(void) {
	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	/* Enables PWM output (of TIM4, channel 4)  */
	palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
	pwmStart(&PWMD4, &pwmcfg_led);
    //Channel here is indexed from 0 instead of 1 as is in the config struct
	pwmEnableChannel(&PWMD4, 3, INIT_PWM_WIDTH_TICS);

    const ShellCommand commands[] = {
            {"pwm", cmd_pwm},
            {NULL, NULL}
    };
    usbSerialShellStart(commands);

	/*
	 * Creates the blinker thread.
	 */
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, led_thread, NULL);
	/*
	 * Creates the pwm thread.
	 */
	chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, pwm_thread, NULL);

	/*
	 * Normal main() thread activity, in this demo it does nothing except
	 * sleeping in a loop
	 */
	while (TRUE) {
        chThdSleep(TIME_INFINITE);
	}
}
