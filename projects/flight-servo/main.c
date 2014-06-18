/*
 * The Roll Control Module is responsible for activating a PWM servo. Commands
 * sent from the FC over ethernet set the pulsewidth.
 */

#include <stdlib.h>
#include <string.h>

// ChibiOS
#include "ch.h"
#include "hal.h"
#include "evtimer.h"

// PSAS common
#include "net_addrs.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "utils_led.h"

/*
 * \warning PERIOD setting for PWM is uint16_t (0-65535)
 *
 * Do not overflow this value! You may have to turn down
 * INIT_PWM_FREQ to obtain pulsewidths you need. See Hitec
 * 20mS example settings.
 * Testing only:
 * Hitec 20mS period (50hZ) servos (Models HS-81 and HS-45HB)
 *  Pulse widths range from 900 uS to 2100 uS

#define         INIT_PWM_FREQ                  3000000
#define         INIT_PWM_PERIOD_TICKS           ((pwmcnt_t)  60000)
#define         INIT_PWM_PULSE_WIDTH_TICKS      ((pwmcnt_t)  4500)
*/
/*
 * Manage the pwm for the servo. We are using the JS DS8717 Servo. Details here:
 * http://www.servodatabase.com/servo/jr/ds8717
 *
 * See this file's corresponding header file for more information on servo
 * limits.
 *
 */
/*
 * Rocket servo
 * 3.3mS period (300hZ)
 * JS DS8717 Servo
 * http://www.servodatabase.com/servo/jr/ds8717
 *
 * Pulse widths range from 1100 uS to 1900 uS
 * Hard stops on mechanism define this range. Center 1500uS pw.
 *
 */
/*! \warning The structure from the RC module on the Flight computer
 * is not packed. Current plans are to pass a byte array from the FC
 * to the RC board. This structure is here for use and reference locally
 * but do not memcpy the network data into it, it may not work.
 */
typedef struct {
    uint16_t u16ServoPulseWidthBin14; // PWM on-time in milliseconds x 2^14
                                      // e.g. 1.5 msec = 1.5 x 2^14 = 24576
    uint8_t u8ServoDisableFlag;       // Disable servo (turn off PWM) when this flag is not 0
} __attribute__((packed)) RCOutput;

/*
 * Servo PWM Constants

 * ===================
 */

#define PWM_CENTER 1500

// Absolute position limits, in microseconds (us).
// Verified empirically by DLP, K, and Dave 3/25/14
#define PWM_LO 1100
#define PWM_HI 1900

// Slew rate limit, in microseconds/millisecond. This limit corresponds to being
// able to go from center to far mechanical limit in 100 ms.
#define PWM_MAX_RATE 5

// Maximum allowable oscillation frequency. We prevent the servo from changing
// direction any faster than this.
#define PWM_MAX_OSCILLATION_FREQ 50
#define PWM_MIN_DIRECTION_CHANGE_PERIOD (1000 / PWM_MAX_OSCILLATION_FREQ)


#define INIT_PWM_FREQ              6000000
#define INIT_PWM_PERIOD_TICKS      ((pwmcnt_t) 20000)
#define INIT_PWM_PULSE_WIDTH_TICKS ((pwmcnt_t) 9000)

#define COMMAND_MAILBOX_SIZE 8


/*
 * Data Structures
 * =============== *************************************************************
 */

typedef struct PositionCommand {
  uint16_t position;
} PositionCommand;

typedef struct PositionDelta {
  uint16_t delta;
} PositionDelta;

// This mailbox is how the thread that listens for position commands over the
// nework sends the commands to the thread that actually sets the PWM output.
// The mailbox stores pointers to PositionCommand structs that are stored in the
// pos_cmd_pool memory pool.
static msg_t servo_command_buffer[COMMAND_MAILBOX_SIZE];
static MAILBOX_DECL(servo_commands, servo_command_buffer, COMMAND_MAILBOX_SIZE);

static MemoryPool pos_cmd_pool;
static PositionCommand pos_cmd_pool_storage[COMMAND_MAILBOX_SIZE] __attribute__((aligned(sizeof(stkalign_t))));

// This mailbox is how the PWM output setter thread sends back deltas between
// commanded position and the actual position that it set.
// The mailbox stores pointers to PositionDelta structs that are stored in the
// pos_delta_pool memory pool.
static msg_t servo_delta_buffer[COMMAND_MAILBOX_SIZE];
static MAILBOX_DECL(servo_deltas, servo_delta_buffer, COMMAND_MAILBOX_SIZE);

static MemoryPool pos_delta_pool;
static PositionDelta pos_delta_pool_storage[COMMAND_MAILBOX_SIZE] __attribute__((aligned(sizeof(stkalign_t))));


/*
 * Function Declarations
 * ===================== *******************************************************
 */

void pwm_start(void);
void pwm_set_pulse_width_ticks(uint32_t width_ticks);
uint32_t pwm_get_period_ms(void);
pwmcnt_t pwm_us_to_ticks(uint32_t us) ;




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


static void set_pwm_position(eventid_t id UNUSED) {

    static int16_t last_difference = 0;
    static uint16_t last_position = 1500;
    static uint32_t ticks = 0;
    static uint32_t last_direction_change_tick = 0;

    ticks++;

    PositionCommand* cmd;
    msg_t fetch_status;

    fetch_status = chMBFetch(&servo_commands, (msg_t *) &cmd, 0);
    if (fetch_status == RDY_TIMEOUT) {
        // if nobody sent a position to our mailbox, we have nothing to do
        return;
    }

    // copy the information out of the struct so we can free it ASAP
    uint16_t desired_position = cmd->position;

    chPoolFree(&pos_cmd_pool, cmd);

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
        uint32_t elapsed = ticks - last_direction_change_tick;
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
        pwmEnableChannel(&PWMD4, 3, pwm_us_to_ticks(output));
    }

    last_position = output;

    if (output != desired_position) {
        PositionDelta* delta = (PositionDelta*) chPoolAlloc(&pos_delta_pool);
        if (delta == NULL) return; // the pool is empty, so bail

        delta->delta = desired_position - output;
        chMBPost(&servo_deltas, (msg_t) delta, 0);
    }
}


WORKING_AREA(wa_listener_thread, 1024);

/*
 * Command listener - sets up a listen socket to receive servo position commands
 * from the flight computer, then posts them to the PWM control thread's
 * mailbox.
 */
static msg_t listener_thread(void* u UNUSED) {
    chRegSetThreadName("command_listener");

    RCOutput rc_packet;
    struct SeqSocket socket = DECL_SEQ_SOCKET(sizeof(RCOutput));

    int s = get_udp_socket(ROLL_ADDR);
    if (s < 0) {
        return -1;
    }

    seq_socket_init(&socket, s);

    while (TRUE) {
        int r;

        r = seq_read(&socket);
        if (r == sizeof(RCOutput)) {
            //fixme: throw away anything left
            memcpy(&rc_packet, socket.buffer, sizeof(RCOutput));

          if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
                PositionCommand* cmd = (PositionCommand*) chPoolAlloc(&pos_cmd_pool);
                if (cmd == NULL) continue; // the pool is empty, so bail until next msg
                cmd->position = ntohs(rc_packet.u16ServoPulseWidthBin14);
                chMBPost(&servo_commands, (msg_t) cmd, TIME_IMMEDIATE);
            } else {
                pwmDisableChannel(&PWMD4, 3);
            }

            PositionDelta* delta;
            msg_t fetch_status = chMBFetch(&servo_deltas, (msg_t *) &delta, TIME_IMMEDIATE);
            if (fetch_status == RDY_TIMEOUT) continue; // no delta received, so wait for next command
            memcpy(socket.buffer, delta, sizeof(PositionDelta));
            seq_write(&socket, sizeof(PositionDelta));
            chPoolFree(&pos_delta_pool, delta);
        }
    }

    return -1;
}

void main(void) {
    /* Initialize Chibios */
    halInit();
    chSysInit();

    /* Diagnostic led */
    ledStart(NULL);

    /* Start lwip stack */
    lwipThreadStart(ROLL_LWIP);

    /* activate PWM output */
    chPoolInit(&pos_cmd_pool, sizeof(PositionCommand), NULL);
    chPoolLoadArray(&pos_cmd_pool, pos_cmd_pool_storage, COMMAND_MAILBOX_SIZE);
    chPoolInit(&pos_delta_pool, sizeof(PositionDelta), NULL);
    chPoolLoadArray(&pos_delta_pool, pos_delta_pool_storage, COMMAND_MAILBOX_SIZE);

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

    EvTimer timer;
    evtInit(&timer, MS2ST(1));

    struct EventListener eltimer;
    static const evhandler_t evhndl[] = {
            set_pwm_position
    };
    chEvtRegister(&timer.et_es, &eltimer, 0);

    evtStart(&timer);
    while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
    }
}
