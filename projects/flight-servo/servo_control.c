/*
 * Manage the pwm for the servo. We are using the JS DS8717 Servo. Details here:
 * http://www.servodatabase.com/servo/jr/ds8717
 *
 * See this file's corresponding header file for more information on servo
 * limits.
 *
 */

// stdlib
#include <stdint.h>
#include <stdlib.h>
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

#define POSITION_MAILBOX_SIZE 8

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
static msg_t servo_position_buffer[POSITION_MAILBOX_SIZE];
static MAILBOX_DECL(servo_positions, servo_position_buffer, POSITION_MAILBOX_SIZE);



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
    chSysLockFromIsr();
    chVTSetI(&servo_output_timer, MS2ST(1), set_pwm_position, 0);
    chSysUnlockFromIsr();

    static int16_t last_difference = 0;
    static uint16_t last_position = 1500;
    static uint64_t ticks = 0;
    static uint64_t last_direction_change_tick = 0;

    ticks++;

    msg_t new_msg, fetch_status;
    chSysLockFromIsr();
    fetch_status = chMBFetchI(&servo_positions, &new_msg);
    chSysUnlockFromIsr();
    if (fetch_status == RDY_TIMEOUT) {
        // if nobody sent a position to our mailbox, we have nothing to do
        return;
    }

    uint16_t desired_position = (uint16_t) new_msg;
    uint16_t output = desired_position;

    int16_t difference = desired_position - last_position;

    // limit rate
    if (abs(difference) > PWM_MAX_RATE) {
        if (difference < 0) {
            output = last_position - PWM_MAX_RATE;
        } else {
            output = last_position + PWM_MAX_RATE;
        }
    }

    // limit oscillation frequency
    if ((difference > 0 && last_difference <= 0)
        || (difference < 0 && last_difference >= 0)) {
        uint64_t elapsed = ticks - last_direction_change_tick;
        if (elapsed < PWM_MIN_DIRECTION_CHANGE_PERIOD) {
            // changed direction too recently; not allowed to change again yet
            output = last_position;
        } else {
            last_direction_change_tick = ticks;
        }
    }

    // hard limits
    if (output < PWM_LO) {
        output = PWM_LO;
    } else if (output > PWM_HI) {
        output = PWM_HI;
    }

    last_difference = output - last_position;
    if (last_difference != 0) {
        pwmEnableChannelI(&PWMD4, 3, pwm_us_to_ticks(output));
    }
    last_position = output;
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

#if DEBUG_PWM
    int16_t ticks = 0;
    while (TRUE) {
        int16_t pos = PWM_CENTER;

        // calculate pos
        if (ticks < 10000) {
            // move from center out to either side and back over two seconds
            int16_t offset = 500 - abs(500 - (ticks % 1000));
            if (ticks % 2000 < 1000) {
                pos = PWM_CENTER + offset;
            } else {
                pos = PWM_CENTER - offset;
            }
        } else if (ticks < 20000) {
            // test speed controls by moving servo as fast as possible
            if (ticks % 2000 < 1000) {
                pos = 1000;
            } else {
                pos = 2000;
            }
        } else {
            ticks = -1;
        }

        // set pos
        chMBPost(&servo_positions, (msg_t) pos, TIME_IMMEDIATE);

        ticks++;
        chThdSleepMilliseconds(1);
    }
#else
    while(TRUE){
        //fixme: throw away anything left
        read(socket, data, sizeof(data));
        memcpy(&rc_packet, data, sizeof(RCOutput));

        // fixme: once the RNH power issue is fixed servo control can respect
        // the disable flag
//        if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
            uint16_t width = rc_packet.u16ServoPulseWidthBin14;

            chMBPost(&servo_positions, (msg_t) width, TIME_IMMEDIATE);

//        } else {
//            pwmDisableChannel(&PWMD4, 3);
//        }
    }
#endif

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

    chVTSet(&servo_output_timer, MS2ST(1), set_pwm_position, 0);
}
