#include <stddef.h>

#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_led.h"

/* Pre-filled led_configs */

#ifdef BOARD_PSAS_ROCKETNET_HUB_1_0
const struct led GREEN = {GPIOD, GPIO_D12_RGB_G};
const struct led RED = {GPIOD, GPIO_D13_RGB_R};
const struct led BLUE = {GPIOD, GPIO_D11_RGB_B};

static struct led_config led_cfg = {
		.cycle_ms = 500,
		.start_ms = 2500,
		.led = (const struct led*[]){
				&GREEN,
				&RED,
				&BLUE,
				NULL
		}
};
#elif defined BOARD_OLIMEX_STM32_E407
const struct led GREEN = {GPIOC, GPIOC_LED};

static struct led_config led_cfg = {
		.cycle_ms = 500,
		.start_ms = 0,
		.led = (const struct led*[]){
				&GREEN,
				NULL
		}
};
#elif defined BOARD_PSAS_GPS_RF_FRONTEND_2
const struct led LED2 = {GPIOD, GPIOD_LED2};
const struct led LED4 = {GPIOD, GPIOD_LED4};
const struct led LED5 = {GPIOD, GPIOD_LED5};
const struct led RED = {GPIOD, GPIOD_RGB_R};
const struct led BLUE = {GPIOD, GPIOD_RGB_B};
const struct led GREEN = {GPIOD, GPIOD_RGB_G};
static struct led_config led_cfg = {
		.cycle_ms = 500,
		.start_ms = 4000,
		.led = (const struct led*[]){
				&GREEN,
				&RED,
				&BLUE,
				&LED2,
				&LED4,
				&LED5,
				NULL
		}
};
#elif defined BOARD_PSAS_RTX_CONTROLLER
const struct led ORANGE = {GPIOE, GPIOE_PIN1};

static struct led_config led_cfg = {
		.cycle_ms = 500,
		.start_ms = 0,
		.led = (const struct led*[]){
				&ORANGE,
				NULL
		}
};
#elif defined BOARD_ST_STM32F4_DISCOVERY
const struct led LED3 = {GPIOD, GPIOD_LED3};
const struct led LED4 = {GPIOD, GPIOD_LED4};
const struct led LED5 = {GPIOD, GPIOD_LED5};
const struct led LED6 = {GPIOD, GPIOD_LED6};

static struct led_config led_cfg = {
		.cycle_ms = 500,
		.start_ms = 4000,
		.led = (const struct led*[]){
				&LED3,
				&LED4,
				&LED5,
				&LED6,
				NULL
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

void ledError(void){
	chMBPost(&mailbox, ERROR, TIME_IMMEDIATE);
}

void ledNominal(void){
	chMBPost(&mailbox, NOMINAL, TIME_IMMEDIATE);
}

void ledOn(const struct led * led){
	if(led){
		palClearPad(led->port, led->pad);
	}
}
void ledOff(const struct led * led){
	if(led){
		palSetPad(led->port, led->pad);
	}
}
void ledToggle(const struct led * led){
	if(led){
		palTogglePad(led->port, led->pad);
	}
}

static WORKING_AREA(wa_led, 512); //fixme mailboxes demand a bizarrely large amount of space
NORETURN static void led(void * arg) {
	struct led_config * cfg = (struct led_config*) arg;
	const struct led **led = cfg->led;

	chRegSetThreadName("LED");

	/* Turn off leds, also count them */
	int num_leds = 0;
	for(; led[num_leds]; ++num_leds){
		// FIXME: they're not all push-pull are they?
		palSetPadMode(led[num_leds]->port, led[num_leds]->pad, PAL_MODE_OUTPUT_PUSHPULL);
		ledOff(led[num_leds]);
	}

	unsigned start_cycles = cfg->start_ms / cfg->cycle_ms;
	systime_t start_blink = cfg->cycle_ms / num_leds;

	/* Run the start pattern */
	int i = 0;
	for(; start_cycles > 0; --start_cycles) {
		for(i = 0; i < num_leds; ++i){
			ledOn(led[i]);
			chThdSleepMilliseconds(start_blink);
			ledOff(led[i]);
		}
		if(cfg->cycle_ms % num_leds){
			chThdSleepMilliseconds(cfg->cycle_ms % num_leds);
		}
	}

	/* Run the toggle pattern */
	int activeled = 0;
	systime_t cycletime = cfg->cycle_ms;
	while (TRUE) {
		ledToggle(led[activeled]);
		msg_t msg = 0;
		chMBFetch(&mailbox, &msg, cycletime);
		switch(msg){
		case NOMINAL:
			cycletime = cfg->cycle_ms;
			activeled = 0;
			break;
		case ERROR:
			cycletime = cfg->cycle_ms/2;
			if(led[1]->port != NULL){
				activeled = 1;
			}
			break;
		}
	}
}

void ledStart(struct led_config * cfg){
	// If cfg is null see if a default one can be found. If it can't it is
	// set to an to an invalid config.
	if(!cfg){
		cfg = &led_cfg;
	}

	//Triggers if cfg isnvalid.
	if(!(cfg->led && cfg->led[0]->port)){
		return; //no defined leds
	}
	chDbgAssert(cfg->cycle_ms > 0, DBG_PREFIX "LED cycle time must be positive", NULL);

	chThdCreateStatic(wa_led, sizeof(wa_led), NORMALPRIO, (tfunc_t)led, cfg);
}

