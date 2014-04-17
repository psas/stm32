#include <stddef.h>

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_led.h"

/* Pre-filled led_configs */

#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0
static struct led_config led_cfg = {
        .cycle_ms = 500,
        .start_ms = 2500,
        .led = (struct pin[]){
                {GPIOD, GPIO_D12_RGB_G},
                {GPIOD, GPIO_D13_RGB_R},
                {GPIOD, GPIO_D11_RGB_B},
                {0, 0}
        }
};
#elif defined BOARD_OLIMEX_STM32_E407
static struct led_config led_cfg = {
        .cycle_ms = 500,
        .start_ms = 0,
        .led = (struct pin[]){
                {GPIOC, GPIOC_LED},
                {NULL, 0}
        }
};
#else
static struct led_config led_cfg = {0};
#endif

#define SIZE 32
static msg_t buffer[SIZE];
MAILBOX_DECL(mailbox, buffer, SIZE);

#define NOMINAL 1
#define ERROR 2

void led_error(void){
    chMBPost(&mailbox, ERROR, TIME_IMMEDIATE);
}

void led_nominal(void){
    chMBPost(&mailbox, NOMINAL, TIME_IMMEDIATE);
}

static WORKING_AREA(wa_led, 512); //fixme mailboxes demand a bizarrely large amount of space
NORETURN static void led(void * arg) {
    struct led_config * cfg = (struct led_config*) arg;
    struct pin *led = cfg->led;

    chRegSetThreadName("LED");

    /* Turn off leds, also count them */
    int num_leds = 0;
    for(; !led[num_leds].port; ++num_leds){
        // FIXME: they're not all push-pull are they?
        palSetPadMode(led[num_leds].port, led[num_leds].pad, PAL_MODE_OUTPUT_PUSHPULL);
        palClearPad(led[num_leds].port, led[num_leds].pad);
    }

    unsigned start_cycles = cfg->start_ms / cfg->cycle_ms;
    systime_t start_blink = cfg->cycle_ms / num_leds;

    /* Run the start pattern */
    int i = 0;
    for(; start_cycles > 0; --start_cycles) {
        for(i = 0; i < num_leds; ++i){
            palSetPad(led[i].port, led[i].pad);
            chThdSleepMilliseconds(start_blink);
            palClearPad(led[i].port, led[i].pad);
        }
        chThdSleepMilliseconds(cfg->cycle_ms % num_leds);
    }

    /* Run the toggle pattern */
    int active = 0;
    systime_t cycletime = cfg->cycle_ms;
    while (TRUE) {
        palTogglePad(led[active].port, led[active].pad);
        msg_t msg = 0;
        chMBFetch(&mailbox, &msg, cycletime);
        switch(msg){
        case NOMINAL:
            cycletime = cfg->cycle_ms;
            active = 0;
            break;
        case ERROR:
            cycletime = cfg->cycle_ms/2;
            if(led[1].port != NULL){
                active = 1;
            }
            break;
        }
    }
}

void led_init(struct led_config * cfg){
    // If cfg is null see if a default one can be found. If it can't it is
    // set to an to an invalid config.
    if(!cfg){
        cfg = &led_cfg;
    }

    //Triggers if cfg isnvalid.
    chDbgAssert(cfg->led && cfg->led[0].port, DBG_PREFIX "No defined LEDs", NULL);
    chDbgAssert(cfg->cycle_ms > 0, DBG_PREFIX "LED cycle time must be positive", NULL);

    chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO, (tfunc_t)led, cfg);
}
