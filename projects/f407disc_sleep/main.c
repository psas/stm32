#include "ch.h"
#include "hal.h"
#include "ext.h"

void					main(void);

static void			appMain(void);
static void			osInit(void);
static void			osShutdown(void);
static void			stopMode(void);

static int	leds[] = {
					GPIOD_LED4,
					GPIOD_LED3,
					GPIOD_LED5,
					GPIOD_LED6
				};

void appMain() {
	int		running;
	uint32_t	i, led, ticks, since;

	// flash all LEDs a few times
	for (i = 0 ; i < 5 ; ++i) {
		for (led = 0 ; led < sizeof(leds) / sizeof(leds[0]) ; ++led)
			palWritePad(GPIOD, leds[led], 1 - (i & 1));

		chThdSleepMilliseconds(100);
	}

	// cycle LED blinking until user button pressed
	ticks = MS2ST(250);
	running = 1;
	led = 0;
	while (running) {
		palTogglePad(GPIOD, leds[led]);
		if (++led == sizeof(leds) / sizeof(leds[0]))
			led = 0;

		since = chTimeNow();
		while (chTimeNow() - since < ticks)
			if (palReadPad(GPIOA, GPIOA_BUTTON)) {
				running = 0;
				break;
			}
	}

	// turn on all LEDs
	for (led = 0 ; led < sizeof(leds) / sizeof(leds[0]) ; ++led)
		palWritePad(GPIOD, leds[led], 1);

/*
	// debounce user button
	ticks = MS2ST(100);
	since = chTimeNow();
	while (chTimeNow() - since < ticks)
		if (palReadPad(GPIOA, GPIOA_BUTTON))
			since = chTimeNow();
*/

	// turn off LEDs one by one, in reverse order from blink cycle
	for (led = sizeof(leds) / sizeof(leds[0]) ; led-- ; ) {
		palWritePad(GPIOD, leds[led], 0);

		chThdSleepMilliseconds(250);
	}
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
}

void osShutdown() {
	chSysDisable();
}

void stopMode() {
	uint32_t	i;

	// configure for STOP mode on WFI
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	PWR->CR &= ~PWR_CR_PDDS;
	PWR->CR |= PWR_CR_FPDS | PWR_CR_LPDS;

	// interrupt 0 source is PA0
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;

	// disable all events
	// enable interrupt 0, disable all others
	// configure interrupt 0 on rising trigger
	// clear all pending interrupts
	for (i = 0 ; i < 23 ; ++i) {
		EXTI->EMR   &= ~(1 << i);
		if (i == 0) {
			EXTI->IMR   |= (1 << i);
			EXTI->RTSR  |= (1 << i);
		} else {
			EXTI->IMR   &= ~(1 << i);
			EXTI->RTSR  &= ~(1 << i);
		}
		EXTI->FTSR  &= ~(1 << i);
		EXTI->PR     =  (1 << i);
	}

	// wait for interrupt
	__WFI();

	// after resuming from stop, the CPU is running on the HSI RC oscillator
	// stm32_clock_init() switches it to the HSE clock, but it is normally
	// called only from crt0, by way of __early_init.
	stm32_clock_init();
}
