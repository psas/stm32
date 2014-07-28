/*
 * Led blinker thread, useful for diagnostics.
 *
 */

#ifndef UTILS_LED_H_
#define UTILS_LED_H_

#include "ch.h"
#include "hal.h"
#include "utils_hal.h"

struct led {
    ioportid_t port;
    uint16_t pad;
};

struct led_config {
    systime_t cycle_ms;   // Main sequence led toggle time, must be positive
    systime_t start_ms;   // Start pattern duration
    const struct led ** led; // Null terminated array of led pointers
};

/* Board dependent physical leds */
#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0
extern const struct led GREEN;
extern const struct led RED;
extern const struct led BLUE;
#elif defined BOARD_OLIMEX_STM32_E407
extern const struct led GREEN;
#elif defined BOARD_PSAS_GPS_RF_FRONTEND_2
extern const struct led GREEN;
extern const struct led RED;
extern const struct led BLUE;
extern const struct led LED2;
extern const struct led LED4;
extern const struct led LED5;
#elif defined BOARD_PSAS_RTX_CONTROLLER
extern const struct led ORANGE;
#elif defined BOARD_ST_STM32F4_DISCOVERY
extern const struct led LED3;
extern const struct led LED4;
extern const struct led LED5;
extern const struct led LED6;
#endif


/* Suggested project code usage:
 * Each project defines a set of logical leds and then maps physical leds
 * to the logical leds in a board dependent way.

logical leds (Defining them in this way lets them default to null):
extern const struct led * behavior_alpha;
extern const struct led * on_bar;

map:
#ifdef BOARD_BAR
const struct led * behavior_alpha = &RED;
const struct led * on_bar = &BLUE;
#elif define FOOBOARD
const struct led * behavior_1 = &GREEN;
on_bar has no corresponding physical led
#endif
*/

/* Functions that operate on LED structures, does nothing if null is passed */
void ledOn(const struct led * led);
void ledOff(const struct led * led);
void ledToggle(const struct led * led);

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
void ledStart(struct led_config * cfg);

/*
 * Indicate an error has occurred.
 * Will cause the led to flash at twice the normal rate and, if it exists,
 * change the blinking led to the second in the list.
 */
void ledError(void);

/*
 * Change led blinking to the default behavior.
 */
void ledNominal(void);

#endif /* UTILS_LED_H_ */
