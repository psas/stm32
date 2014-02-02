#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "usbdetail.h"
#include "iwdg_lld.h"

#include "debug_pwm.h"
#include "servo_control.h"

#define DEBUG_PWM 1
#if DEBUG_PWM

#define UNUSED __attribute__((unused))

//static WORKING_AREA(wa_watchdog_keeper, 128);
static WORKING_AREA(wa_pwm_tester, 512);

static int pwm_lo = 1420;
static int pwm_hi = 1620;

void cmd_pwmlims(BaseSequentialStream *out, int argc, char* argv[]) {
    if (argc == 0) {
        chprintf(out, "lo: %i\thi: %i\r\n", pwm_lo, pwm_hi);
        return;
    }

    if (argc != 2) {
        chprintf(out, "usage: pwm [lo hi]\r\n");
        return;
    }

    int lo = atoi(argv[0]);
    int hi = atoi(argv[1]);

    if (lo > hi) {
        chprintf(out, "ERROR: lo limit (%i) is greater than hi limit (%i)\r\n", lo, hi);
        return;
    }

    pwm_lo = lo;
    pwm_hi = hi;
}


/*
 * Test PWM Thread
 *
 * Steps up and then down through servo positions.
 */

static msg_t pwm_tester(void *_ UNUSED) {

    chRegSetThreadName("pwmtest");
    BaseSequentialStream *chp = getUsbStream();
    chThdSleepMilliseconds(1000);

    while(1) {
        int32_t pulse = 0;

        for (pulse = pwm_lo; pulse <= pwm_hi; pulse += 50) {
            pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
            chprintf(chp, "%d\r\n", pulse);
            chThdSleepMilliseconds(50);
        }

        chprintf(chp, "rest\r\n");
        chThdSleepMilliseconds(5000);

        for (pulse = pwm_hi; pulse >= pwm_lo; pulse -= 50) {
            pwm_set_pulse_width_ticks(pwm_us_to_ticks(pulse));
            chprintf(chp, "%d\r\n", pulse);
            chThdSleepMilliseconds(50);
        }

        chprintf(chp, "rest\r\n");
        chThdSleepMilliseconds(5000);

    }
    return -1;
}

void debug_pwm_start(void){
    chThdCreateStatic( wa_pwm_tester
                     , sizeof(wa_pwm_tester)
                     , NORMALPRIO
                     , pwm_tester
                     , NULL
                     );


    static const ShellCommand commands[] = {
        {"pwm", cmd_pwmlims},
        {NULL, NULL}
    };

    usbSerialShellStart(commands);
}

#endif /*DEBUG_PWM*/
