/*
 * Led blinker threads, useful for diagnostics.
 *
 */

#ifndef UTILS_LED_H_
#define UTILS_LED_H_

#include "ch.h"
#include "hal.h"
#include "utils_hal.h"

struct led_config {
    systime_t cycle_ms;   // Main sequence led toggle time
    systime_t start_ms;   // Start pattern duration
    struct pin led[]; // struct pin {0, 0} terminated list of led pins
};


/*
 * Starts a thread that will:
 * 1. turn off all given leds.
 * 2. cycle through all leds for start_ms milliseconds. start_ms should be
 *    greater than cycle_ms.
 * 3. then toggle the first led in the list every cycle_ms milliseconds.
 */
void led_init(struct led_config * cfg);

/* Pre-filled led configs */

#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0
extern struct led_config rnh_led_cfg;
#endif

#ifdef BOARD_OLIMEX_STM32_E407
extern struct led_config e407_led_cfg;
#endif

#endif /* UTILS_LED_H_ */
