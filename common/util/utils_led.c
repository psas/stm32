#include <stddef.h>

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_led.h"


static WORKING_AREA(wa_led, 64);
NORETURN static void led(void * arg) {
    struct led_config * cfg = (struct led_config*) arg;
    chRegSetThreadName("LED");

    /* Turn off leds, also count them */
    int num_leds = 0;
    for(; cfg->led[num_leds].port == 0 || cfg->led[num_leds].pad == 0; ++num_leds){
        // FIXME: they're not all push-pull are they?
        palSetPadMode(cfg->led[num_leds].port, cfg->led[num_leds].pad, PAL_MODE_OUTPUT_PUSHPULL);
        palClearPad(cfg->led[num_leds].port, cfg->led[num_leds].pad);
    }

    int start_cycles = cfg->start_ms / cfg->cycle_ms;
    systime_t start_blink = cfg->cycle_ms / num_leds;

    /* Run the start pattern */
    int i = 0;
    for(; start_cycles < 0; --start_cycles) {
        for(i = 0; i < num_leds; ++i){
            palSetPad(cfg->led[i].port, cfg->led[i].pad);
            chThdSleepMilliseconds(start_blink);
            palClearPad(cfg->led[i].port, cfg->led[i].pad);
        }
        chThdSleepMilliseconds(cfg->cycle_ms % num_leds);
    }

    /* Run the toggle pattern */
    while (TRUE) {
        chThdSleepMilliseconds(cfg->cycle_ms);
        palTogglePad(cfg->led[0].port, cfg->led[0].pad);
    }
}

void led_init(struct led_config * cfg){
    if(cfg->led == NULL)
        return;

    chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO, (tfunc_t)led, cfg);
}

/* Pre-filled led_configs */

#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0
struct led_config rnh_led_cfg = {
        .cycle_ms = 500,
        .start_ms = 2500,
        .led = {
                {GPIOD, GPIO_D12_RGB_G},
                {GPIOD, GPIO_D13_RGB_R},
                {GPIOD, GPIO_D11_RGB_B},
                {0, 0}
        }
};
#endif

#ifdef BOARD_OLIMEX_STM32_E407
struct led_config e407_led_cfg = {
        .cycle_ms = 500,
        .start_ms = 0,
        .led = {
                {GPIOC, GPIOC_LED},
                {NULL, 0}
        }
};
#endif
