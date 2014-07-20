/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth.
 *
 * Rocket servo
 * 3.3mS period (300hZ)
 * JS DS8717 Servo
 * http://www.servodatabase.com/servo/jr/ds8717
 */

#include <stdlib.h>
#include <string.h>

// ChibiOS
#include "ch.h"
#include "hal.h"

// PSAS common
#include "net_addrs.h"
#include "rci.h"
#include "utils_rci.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_led.h"

#ifdef FLIGHT
#include "iwdg.h"
#endif
/*
 * Servo PWM Constants
 * ===================
 */
// These two values were chosen such that  the PWM frequency is at
// exactly 300hz and PWM_PERIOD_TICKS is as large as possible
// The equation is freq = PWM_CLK / PWM_PERIOD_TICKS
// and PWM_FREQ_HZ must divide 84000000 (timer clock)
#define PWM_PERIOD_TICKS 56000
#define PWM_CLK 16800000
#define PWM_FREQ (PWM_CLK / PWM_PERIOD_TICKS)

// Absolute position limits, in microseconds (us).
// Verified empirically by DLP, K, and Dave 3/25/14
#define PWM_LO_US 1100
#define PWM_HI_US 1900
// Limits in ticks. Division by 1e6 is broken up here to avoid over/underflows
#define PWM_LO ((PWM_LO_US * PWM_FREQ) / 1000 * PWM_PERIOD_TICKS / 1000)
#define PWM_HI ((PWM_HI_US * PWM_FREQ) / 1000 * PWM_PERIOD_TICKS / 1000)
#define PWM_CENTER (PWM_HI + PWM_LO) / 2

#ifndef FLIGHT
// Slew rate limit, in microseconds/millisecond. This limit corresponds to being
// able to go from center to far mechanical limit in 100 ms.
#define PWM_MAX_RATE 5

// Maximum allowable oscillation frequency. We prevent the servo from changing
// direction any faster than this.
#define PWM_MAX_OSCILLATION_FREQ 50
#define PWM_MIN_DIRECTION_CHANGE_PERIOD (PWM_FREQ / PWM_MAX_OSCILLATION_FREQ)
#endif

/*
 * Global Variables
 * ================ ***********************************************************
 */
typedef struct {
	uint32_t seqCounter; // Packet sequence counter
	uint16_t pulseWidth; // PWM on-time in milliseconds x 2^14 e.g. 1.5 msec = 1.5 x 2^14 = 24576
	uint8_t disableFlag; // Disable servo (turn off PWM) when this flag is not 0
} __attribute__((packed)) RCCommand;

typedef struct {
	uint32_t seqCounter; // Packet sequence counter
	uint32_t seqError;   // Sequence number of error packet
	uint16_t pwmError;   // Value PWM has been set to instead
} __attribute__((packed)) RCError;

static uint16_t softstop(uint16_t position){
	if (position < PWM_LO) {
		return PWM_LO;
	}
	if (position > PWM_HI) {
		return PWM_HI;
	}
	return position;
}

#ifndef FLIGHT
static uint16_t last_last_position = PWM_CENTER;
static uint16_t last_position = PWM_CENTER;
static uint16_t last_command = PWM_CENTER;
static int ticks;

static uint16_t ratelimit(uint16_t position){
	int32_t difference = position - last_position;
	if (abs(difference) > PWM_MAX_RATE) {
		if (difference < 0) {
			return last_position - PWM_MAX_RATE;
		} else {
			return last_position + PWM_MAX_RATE;
		}
	}
	return position;
}

static uint16_t oscillationlimit(uint16_t position){
	static int last_direction_change_tick = 0;
	int32_t difference = position - last_position;
	int32_t last_difference = last_position - last_last_position;
	if (   (difference > 0 && last_difference <= 0)
	    || (difference < 0 && last_difference >= 0)) {
		uint32_t elapsed = ticks - last_direction_change_tick;
		if (elapsed < PWM_MIN_DIRECTION_CHANGE_PERIOD) {
			// changed direction too recently; not allowed to change again yet
			return last_position;
		} else {
			last_direction_change_tick = ticks;
			return position;
		}
	}
	return position;
}

static uint16_t positionlimit(uint16_t position){
	position = softstop(position);
	position = ratelimit(position);
	position = oscillationlimit(position);
	return position;
}

static void pwmcallback(PWMDriver * driver UNUSED){
	last_last_position = last_position;
	last_position = PWMD4.tim->CCR[3];
	++ticks;

	pwmEnableChannelI(&PWMD4, 3, positionlimit(last_command));
}
#else
#define pwmcallback NULL
#endif

static void pwdg_handler(void * p UNUSED){
	chSysLockFromIsr();
	pwmEnableChannelI(&PWMD4, 3, PWM_CENTER);
	chSysUnlockFromIsr();
}

static VirtualTimer pwdg; //Packet WatchDoG

static void handle_command(int socket, RCCommand * packet){
	chVTReset(&pwdg);
	if(packet->disableFlag){
		pwmDisableChannel(&PWMD4, 3);
		return;
	}
	chVTSet(&pwdg, S2ST(1), pwdg_handler, NULL);

	uint16_t position = packet->pulseWidth;
	position = softstop(position);
#ifndef FLIGHT
	last_command = packet->pulseWidth;
	position = ratelimit(position);
	position = oscillationlimit(position);
#endif
	pwmEnableChannel(&PWMD4, 3, position);

	if (position != packet->pulseWidth) {
		static uint32_t sendCounter = 0;
		RCError error;
		error.seqCounter = htonl(sendCounter);
		error.seqError = htonl(packet->seqCounter);
		error.pwmError = htons(position);
		write(socket, &error, sizeof(error));
		++sendCounter;
	}
}

void main(void) {
	halInit();
	chSysInit();
#ifdef FLIGHT
	iwdgStart();
#endif
	ledStart(NULL);

	lwipThreadStart(ROLL_LWIP);

	struct RCICommand commands[] = {
		RCI_CMD_VERS,
		{NULL}
	};
	RCICreate(commands);

	/* Configure PWM. Static because pwmStart doesn't deep copy PWMConfigs */
	static PWMConfig pwmcfg = {
		.frequency = PWM_CLK,
		.period = PWM_PERIOD_TICKS,
		.callback = pwmcallback,
		.channels = {
			{PWM_OUTPUT_DISABLED, NULL},
			{PWM_OUTPUT_DISABLED, NULL},
			{PWM_OUTPUT_DISABLED, NULL},
			{PWM_OUTPUT_ACTIVE_HIGH, NULL}, /* Only channel 4 enabled, PD15 (Physical pin 18) */
		},
		.cr2 = 0
	};
	palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
	pwmStart(&PWMD4, &pwmcfg);

	int s = get_udp_socket(ROLL_ADDR);
	chDbgAssert(s >= 0, "Couldn't get roll socket", NULL);

	RCCommand packet;
	uint32_t recvCounter = 0;
	while (TRUE) {
		int r = read(s, &packet, sizeof(RCCommand));
		if (r == sizeof(RCCommand)) {
			packet.seqCounter = ntohl(packet.seqCounter);
			packet.pulseWidth = ntohs(packet.pulseWidth);
			if(packet.seqCounter > recvCounter){
				handle_command(s, &packet);
			}
			recvCounter = packet.seqCounter;
		}
	}
}

