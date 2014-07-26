#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_led.h"

static volatile bool buttonFlag;

static void irqButton(EXTDriver* extp UNUSED, expchannel_t channel UNUSED) {
	buttonFlag = true;
}

static const EXTConfig extConfig =
{
	{
		{ EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, irqButton },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL },
		{ EXT_CH_MODE_DISABLED, NULL }
	}
};


static void appMain(void) {
	unsigned int i, led = 0;

	struct led const * const leds[] = {
		&LED4,
		&LED3,
		&LED5,
		&LED6
	};
	unsigned int numleds = sizeof(leds)/sizeof(leds[0]);

	// flash all LEDs a few times
	for (i = 0 ; i < 5 ; ++i) {
		for (led = 0 ; led < numleds ; ++led)
			ledToggle(leds[led]);
		chThdSleepMilliseconds(100);
	}

	// cycle LED blinking until user button pressed
	buttonFlag = false;
	while (!buttonFlag) {
		ledToggle(leds[led++]);
		led = led % numleds;
		chThdSleepMilliseconds(250);
	}

	// turn on all LEDs
	for (led = 0 ; led < numleds ; ++led)
		ledOn(leds[led]);

	// turn off LEDs one by one, in reverse order from blink cycle
	for (led = numleds ; led-- ; ) {
		ledOff(leds[led]);
		chThdSleepMilliseconds(250);
	}
}

static void osInit(void) {
	halInit();
	chSysInit();
	extStart(&EXTD1, &extConfig);
	extChannelEnable(&EXTD1, 0);
}

static void osShutdown(void) {
	chSysDisable();
}

static void stopMode(void) {
	// configure for STOP mode on WFI
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	PWR->CR &= ~PWR_CR_PDDS;
	PWR->CR |= PWR_CR_FPDS | PWR_CR_LPDS;

	// wait for interrupt
	__WFI();

	// after resuming from stop, the CPU is running on the HSI RC oscillator
	// stm32_clock_init() switches it to the HSE clock, but it is normally
	// called only from crt0, by way of __early_init.
	stm32_clock_init();

	extStop(&EXTD1);
}

void main(void) {
	while (TRUE) {
		osInit();
		appMain();
		osShutdown();
		stopMode();
	}
}
