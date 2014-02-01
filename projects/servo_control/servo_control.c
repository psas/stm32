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

// PSAS
#include "device_net.h"
#include "psas_packet.h"
#include "servo_control.h"

#define UNUSED __attribute__((unused))

/*
 * Constant Definitions
 * ==================== ********************************************************
 */

#define         INIT_PWM_FREQ                  6000000
#define         INIT_PWM_PERIOD_TICKS           ((pwmcnt_t) 20000)
#define         INIT_PWM_PULSE_WIDTH_TICKS      ((pwmcnt_t) 9000)

// This enum corresponds to the PWM bisable bit in the message received from the
// flight computer. Since it's a _disable_ bit, when it is zero then the PWM is
// enabled.
enum {
    PWM_ENABLE = 0,
    PWM_DISABLE
};


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
    double   ticks_per_us_quot;
    uint32_t ticks_per_us;

    ticks_per_us_quot = INIT_PWM_FREQ/1e6;
    ticks_per_us      = (pwmcnt_t) ticks_per_us_quot;

    return (pwmcnt_t) (us * ticks_per_us);
}


/*
 * This is the inner loop of the data_udp_rx_thread, wherein we read a Roll
 * Control message from the network and adjust the PWM output according to its
 * instructions.
 */
static void read_roll_ctl_and_adjust_pwm(int socket) {
    RCOutput       rc_packet;
    char data[sizeof(RCOutput)];

    //fixme: throw away anything left
    read(socket, data, sizeof(data));
    memcpy(&rc_packet, data, sizeof(RCOutput));

    //fixme: numerical stability of doubles
    if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
        uint16_t width = rc_packet.u16ServoPulseWidthBin14;
//        double   ms_d  = width/pow(2,14);
//        double   us_d  = ms_d * 1000;

        pwmEnableChannel(&PWMD4, 3, pwm_us_to_ticks(width));
    } else {
        pwmDisableChannel(&PWMD4, 3);
    }

}

/*
 * This is the other part of the data_udp_rx_thread, which wraps the
 * read_roll_ctl_and_adjust_pwm routine above. It sets up the UDP connection,
 * bailing with a RDY_RESET msg if binding doesn't succeed; otherwise it loops
 * the above routine forever.
 */
WORKING_AREA(wa_data_udp_rx_thread, 1024);

msg_t data_udp_rx_thread(void * u UNUSED) {
    chRegSetThreadName("data_udp_receive_thread");
    //FIXME: use common code networking
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET,
    addr.sin_port = htons(ROLL_CTL_LISTEN_PORT);
    inet_aton(ROLL_CTL_IP_ADDR_STRING , &addr.sin_addr);

    int pwm_socket = socket(AF_INET,  SOCK_DGRAM, 0);
    bind(pwm_socket, (struct sockaddr*)&addr, sizeof(addr));

    while(TRUE){
        read_roll_ctl_and_adjust_pwm(pwm_socket);
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
    pwmEnableChannel(&PWMD4, 3, INIT_PWM_PULSE_WIDTH_TICKS);
    pwmDisableChannel(&PWMD4, 3);

//    chThdCreateStatic( wa_data_udp_rx_thread
//                     , sizeof(wa_data_udp_rx_thread)
//                     , NORMALPRIO
//                     , data_udp_rx_thread
//                     , NULL
//                     );
}
