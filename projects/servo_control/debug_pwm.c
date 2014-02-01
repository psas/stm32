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
/*
 * Global Variables
 * ================ ************************************************************
 */

/*
 * Threads
 * ======= *********************************************************************
 */
/*
 * Constant Definitions
 * ==================== ********************************************************
 */


//static WORKING_AREA(wa_watchdog_keeper, 128);
static WORKING_AREA(wa_pwm_tester, 512);

static int pwm_lo = 1420;
static int pwm_hi = 1620;

void cmd_mem(BaseSequentialStream *out, int _, char *__[]) {
    (void)_;
    (void)__;

    size_t fragments, size;

    fragments = chHeapStatus(NULL, &size);
    chprintf(out, "core free memory : %u bytes\r\n", chCoreStatus());
    chprintf(out, "heap fragments   : %u\r\n", fragments);
    chprintf(out, "heap free total  : %u bytes\r\n", size);
}


void cmd_threads(BaseSequentialStream *out, int _, char *__[]) {
    (void)_;
    (void)__;

    static const char *THREAD_STATES[] = {THD_STATE_NAMES};
    Thread *t;

    chprintf(out, "addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");

    t = chRegFirstThread();
    do {
        chprintf( out
                , "%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n"
                , (uint32_t)t               // address
                , (uint32_t)t->p_ctx.r13    // stack
                , (uint32_t)t->p_prio       // prio
                , (uint32_t)(t->p_refs - 1) // refs
                , THREAD_STATES[t->p_state] // state
                , (uint32_t)t->p_time       // time
                , t->p_name                 // name
                );
        t = chRegNextThread(t);
    } while (t != NULL);
}


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

static msg_t pwm_tester(void *_) {
    (void)_;

    chRegSetThreadName("pwmtest");
    BaseSequentialStream *chp = getActiveUsbSerialStream();
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


/*
 * Watchdog Thread
 *
 * Keep the watchdog at bay - we power cycle if this thread doesn't run.
 */
//static msg_t watchdog_keeper(void *_) {
//    (void)_;
//
//    chRegSetThreadName("iwatchdog");
//
//    while (TRUE) {
//        iwdg_lld_reload();
//        chThdSleepMilliseconds(250);
//    }
//    return -1;
//}

void debug_pwm_start(void){
    // start the watchdog timer
//    iwdg_begin();
//    chThdCreateStatic( wa_watchdog_keeper
//                     , sizeof(wa_watchdog_keeper)
//                     , NORMALPRIO
//                     , watchdog_keeper
//                     , NULL
//                     );


    chThdCreateStatic( wa_pwm_tester
                     , sizeof(wa_pwm_tester)
                     , NORMALPRIO
                     , pwm_tester
                     , NULL
                     );


    static const ShellCommand commands[] = {
        {"mem", cmd_mem},
        {"threads", cmd_threads},
        {"pwm", cmd_pwmlims},
        {NULL, NULL}
    };

    usbSerialShellStart(commands);
}

#endif /*DEBUG_PWM*/
