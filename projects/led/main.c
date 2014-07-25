/* Basic LED blinking demo. Good for checking that your toolchain works. */

#include "ch.h"
#include "hal.h"

/* Structure to abstract the LED pin configuration. In a real project this is
 * handled for you by utils_led, but it's defined here to keep dependencies at
 * a minimum.
 */
struct led {
	ioportid_t port;
	uint16_t pad;
};

/* Conditional definition of LED to allow for multiple supported boards by
 * the same project
 */
extern const struct led LED;
#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0 /* PSAS Rocketnet Hub board */
const struct led LED = {GPIOD, GPIO_D12_RGB_G};
#elif defined BOARD_OLIMEX_STM32_E407 /* Olimex e407 board */
const struct led LED = {GPIOC, GPIOC_LED};
#elif defined BOARD_PSAS_RTX_CONTROLLER /* PSAS RTX Controller */
const struct led LED = {GPIOE, GPIOE_PIN1};
#endif


/* Blinks the LED */
static void blink(void) {
	while (1) {
		palTogglePad(LED.port, LED.pad);
		chThdSleepMilliseconds(500);
	}
}

/* Application entry point */
void main(void) {
	/* System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations. Config files for these
	 *   are located in halconf.h and mcuconf.h
	 */
	halInit();
	/* - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active. The kernel config file is located in chconf.h
	 */
	chSysInit();

	/* Normal main() thread activity, unendingly blinks an LED */
	blink();

	/* main() should never return */
	while(TRUE){
		chThdSleep(TIME_INFINITE);
	}
}

