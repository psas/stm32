#include "ch.h"
#include "hal.h"
#include "ext.h"

void					main(void);

static void			appMain(void);
static void			irqButton(EXTDriver* extp, expchannel_t channel);
static void			osInit(void);
static void			osShutdown(void);
static void			stopMode(void);

static volatile int	buttonFlag;

static const EXTConfig	extConfig = {
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

static int	leds[] = {
					GPIOD_LED4,
					GPIOD_LED3,
					GPIOD_LED5,
					GPIOD_LED6
				};

void appMain() {
	uint32_t	i, led;

	// flash all LEDs a few times
	for (i = 0 ; i < 5 ; ++i) {
		for (led = 0 ; led < sizeof(leds) / sizeof(leds[0]) ; ++led)
			palWritePad(GPIOD, leds[led], 1 - (i & 1));

		chThdSleepMilliseconds(100);
	}

	// cycle LED blinking until user button pressed
	buttonFlag = 0;
	led = 0;
	while (!buttonFlag) {
		palTogglePad(GPIOD, leds[led]);
		if (++led == sizeof(leds) / sizeof(leds[0]))
			led = 0;

		chThdSleepMilliseconds(250);
	}

	// turn on all LEDs
	for (led = 0 ; led < sizeof(leds) / sizeof(leds[0]) ; ++led)
		palWritePad(GPIOD, leds[led], 1);

	// turn off LEDs one by one, in reverse order from blink cycle
	for (led = sizeof(leds) / sizeof(leds[0]) ; led-- ; ) {
		palWritePad(GPIOD, leds[led], 0);

		chThdSleepMilliseconds(250);
	}
}

void irqButton(EXTDriver* extp, expchannel_t channel) {
	(void)extp;
	(void)channel;

	buttonFlag = true;
}

void main(void) {
	while (1) {
		osInit();

		appMain();

		osShutdown();

		stopMode();
	}
}

void osInit() {
	halInit();
	chSysInit();
	extStart(&EXTD1, &extConfig);
	extChannelEnable(&EXTD1, 0);
}

void osShutdown() {
	chSysDisable();
}

void stopMode() {
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
