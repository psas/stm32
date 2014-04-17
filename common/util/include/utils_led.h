/*
 * Led blinker thread, useful for diagnostics.
 *
 */

#ifndef UTILS_LED_H_
#define UTILS_LED_H_

#include "ch.h"
#include "hal.h"
#include "utils_hal.h"

struct led_config {
    systime_t cycle_ms;   // Main sequence led toggle time, must be positive
    systime_t start_ms;   // Start pattern duration
    struct pin * led;     // Null terminated array of led pins,
                          // must have at least one valid pin.
};


/*
 * Starts a thread that will:
 * 1. turn off all given leds.
 * 2. cycle through all leds for start_ms milliseconds. start_ms should be
 *    greater than cycle_ms.
 * 3. then toggle the first led in the list every cycle_ms milliseconds.
 *
 * If the argument to led_init is null it will attempt to find a configuration
 * suitable for the board it was compiled for. Otherwise it will raise an
 * assertion. Known boards:
 * BOARD_PSAS_ROCKETNET_HUB_1_0
 * BOARD_OLIMEX_STM32_E407
 */
void led_init(struct led_config * cfg);

/*
 * Indicate an error has occurred.
 * Will cause the led to flash at twice the normal rate and, if it exists,
 * change the blinking led to the second in the list.
 */
void led_error(void);

/*
 * Change led blinking to the default behavior.
 */
void led_nominal(void);

#endif /* UTILS_LED_H_ */
