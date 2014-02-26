/*
 * Manage the pwm for the servo. We are using the JS DS8717 Servo. Details here:
 * http://www.servodatabase.com/servo/jr/ds8717
 *
 * The pulse widths on our servo range from 333 us to 1520 us.
 * The usable roll control range on our servo is between 1000 us to 2000 us,
 * with the center sensibly at 1500 us.
 * FIXME: is it really? I found *three* different definitions of the servo
 * range:
 *   1). Comments near the top of this file said "from 333 us to 1520 us" (right
 *       after a link to the servo's datasheet).
 *   2). Comments near the top of this file's header said that "for testing
 *       only" the range is "from 900 us to 2100 us".
 *   3). Comments also in the header, but after those referenced in 2 define the
 *       range to be 1000 us to 2000us, and claims these are enforced by
 *       mechanical stops, so that's what I took.
 *
 * Attempt to standardize units on 'ticks' of PWM module. One PWM clock is one
 * 'tick'.
 */

// stdlib
#include <stdint.h>
#include <string.h>

// ChibiOs
#include "ch.h"
#include "hal.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "chprintf.h"

// PSAS
#include "usbdetail.h"
#include "net_addrs.h"
#include "utils_sockets.h"
#include "psas_packet.h"
#include "servo_control.h"

#define UNUSED __attribute__((unused))

/*
 * Constant Definitions
 * ==================== ********************************************************
 */

#define INIT_PWM_FREQ              6000000
#define INIT_PWM_PERIOD_TICKS      ((pwmcnt_t) 20000)
#define INIT_PWM_PULSE_WIDTH_TICKS ((pwmcnt_t) 9000)

#define COMMAND_MAILBOX_SIZE 8
#define COMMAND_MOVING_AVERAGE_SAMPLES 10

// This enum corresponds to the PWM bisable bit in the message received from the
// flight computer. Since it's a _disable_ bit, when it is zero then the PWM is
// enabled.
enum {
    PWM_ENABLE = 0,
    PWM_DISABLE
};

/*
 * Global Variables
 * ================ ***********************************************************
 */

static struct VirtualTimer servo_output_timer;

// This mailbox is how the thread that receives servo position commands from the
// flight computer sends them to the thread that actually sets PWM output.
static msg_t servo_command_buffer[COMMAND_MAILBOX_SIZE];
static MAILBOX_DECL(servo_commands, servo_command_buffer, COMMAND_MAILBOX_SIZE);



void pwm_set_pulse_width_ticks(uint32_t ticks){
    pwmEnableChannel(&PWMD4, 3, ticks);
}

/*
 * This is a conversion function to return the period in ms.
 */
uint32_t pwm_get_period_ms() {
    return (uint32_t) (PWMD4.tim->CCR[3]/PWMD4.period * 1000) ;
}

/*
 * Convert from microseconds to PWM ticks (1 second = PWM_FREQ ticks)
 */
pwmcnt_t pwm_us_to_ticks(uint32_t us) {
    uint32_t ticks_per_us = INIT_PWM_FREQ / 1000000;
    return (pwmcnt_t) (us * ticks_per_us);
}


static void set_pwm_position(void* u UNUSED) {
    chVTSetI(&servo_output_timer, MS2ST(1), set_pwm_position, 0);

    static uint16_t command_history[COMMAND_MOVING_AVERAGE_SAMPLES] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    msg_t new_position;
    if (chMBFetchI(&servo_commands, &new_position) == RDY_TIMEOUT) {
        return;
    }

    int i;
    // shift values towards the end of the array
    for (i = COMMAND_MOVING_AVERAGE_SAMPLES - 1; i > 0; i--) {
        command_history[i] = command_history[i-1];
    }
    command_history[i] = (uint16_t) new_position;

    uint32_t sum = 0;
    for (i = 0; i < COMMAND_MOVING_AVERAGE_SAMPLES; i++) {
        sum += command_history[i];
    }
    uint32_t output_pos = sum / COMMAND_MOVING_AVERAGE_SAMPLES;

    if (output_pos < PWM_LO) {
        output_pos = PWM_LO;
    } else if (output_pos > PWM_HI) {
        output_pos = PWM_HI;
    }

    pwmEnableChannelI(&PWMD4, 3, pwm_us_to_ticks(output_pos));
}


WORKING_AREA(wa_listener_thread, 1024);

/*
 * Command listener - sets up a listen socket to receive servo position commands
 * from the flight computer, then posts them to the PWM control thread's
 * mailbox.
 */
static msg_t listener_thread(void* u UNUSED) {
    chRegSetThreadName("command_listener");

    RCOutput       rc_packet;
    char data[sizeof(RCOutput)];

    int socket = get_udp_socket(ROLL_ADDR);
    if(socket < 0){
        return -1;
    }

    while(TRUE){
        //fixme: throw away anything left
        read(socket, data, sizeof(data));
        memcpy(&rc_packet, data, sizeof(RCOutput));

        // fixme: once the RNH power issue is fixed servo control can respect
        // the disable flag
//        if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
            uint16_t width = rc_packet.u16ServoPulseWidthBin14;

            chMBPost(&servo_commands, (msg_t) width, TIME_IMMEDIATE);

//        } else {
//            pwmDisableChannel(&PWMD4, 3);
//        }
    }
    return -1;
}

void pwm_start() {
    // Static because pwmStart doesn't deep copy PWMConfigs
    static PWMConfig pwmcfg = {
        .frequency = INIT_PWM_FREQ, /* 6Mhz PWM clock frequency; (1/f) = 'ticks' => 167ns/tick */
        .period = INIT_PWM_PERIOD_TICKS, /* PWM period_ticks in ticks;
                                 (T) in ticks * us/tick = 20000 ticks * 167nS/tick = 3.34mS period =>  300 Hz  */
        .callback = NULL,                 /* No callback */
        .channels = {
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_DISABLED, NULL},
                {PWM_OUTPUT_ACTIVE_HIGH, NULL},   /* Only channel 4 enabled, PD15 (Physical pin 18) */
        },
        .cr2 = 0
    };

    palSetPadMode(GPIOD, GPIOD_PIN15, PAL_MODE_ALTERNATE(2));
    pwmStart(&PWMD4, &pwmcfg);
    // Channel is enabled here to reduce inrush current of the servo turning
    // on from browning out the RNH when the flight computer is on.
    // Once the RNH is fixed the channel can probably stay off until asked
    // over ethernet.
    pwmEnableChannel(&PWMD4, 3, INIT_PWM_PULSE_WIDTH_TICKS);

    chThdCreateStatic( wa_listener_thread
                     , sizeof(wa_listener_thread)
                     , NORMALPRIO
                     , listener_thread
                     , NULL
                     );

    chVTSetI(&servo_output_timer, MS2ST(1), set_pwm_position, 0);
}
