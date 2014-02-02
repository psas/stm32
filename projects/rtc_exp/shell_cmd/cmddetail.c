/*! \file cmddetail.c
 *
 */

/*!
 * \defgroup cmddetail Command Utilities
 * @{
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "mac.h"

#include "chrtclib.h"

#include "psas_rtc.h"
#include "cmddetail.h"

/*! test the psas_rtc API
 */
void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)argv;
    static time_t      unix_time;
    struct tm timp;
    RTCTime   psas_time;

    if (argc == 0) {
        goto ERROR;
    }
    if ((argc == 1) && (strcmp(argv[0], "get") == 0)) {
        psas_rtc_lld_get_time(&RTCD1, &psas_time);
        unix_time = psas_rtc_dr_tr_to_unixtime(&psas_time);

        // psas_stm32_rtc_bcd2tm(&timp, &psas_time);
        // unix_time = mktime(&timp);

        if (unix_time == -1){
            chprintf(chp, "incorrect time in RTC cell\r\n");
        } else {
            chprintf(chp, "%Ds %Dus %s",unix_time, psas_time.tv_msec, " - unix time\r\n");
            rtcGetTimeTm(&RTCD1, &timp);
            chprintf(chp, "%s%s",asctime(&timp)," - formatted time string\r\n");
        }
        return;
    } else if ((argc == 1) && (strcmp(argv[0], "test") == 0)) {
        int         ret;
        psas_time.tv_time  = 1382142229;
        psas_time.tv_msec  = 100;

        psas_rtc_lld_set_time(&RTCD1, &psas_time);

        chThdSleepMilliseconds(300);
        psas_rtc_lld_get_time(&RTCD1, &psas_time);
        chprintf(chp, "ms: %u\r\n", psas_time.tv_msec);

        ret       = psas_rtc_get_unix_time(&RTCD1, &psas_time);

        unix_time = psas_time.tv_time;
        if (ret == -1) {
            chprintf(chp, "incorrect time in RTC cell\r\n");
        } else {
            chprintf(chp, "Test: %Ds %Dus %s",unix_time, psas_time.tv_msec, " - unix time\r\n");
        }
        return;
    } else if (argc==1) {
        goto ERROR;
    } else {
        ;
    }

    if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
        unix_time = atol(argv[1]);
        if (unix_time > 0) {
            rtcSetTimeUnixSec(&RTCD1, unix_time);
            return;
        }
        else {
            goto ERROR;
        }
    } else {
        goto ERROR;
    }

ERROR:
    chprintf(chp, "Usage: date get\r\n");
    chprintf(chp, "       date test\r\n");
    chprintf(chp, "       date set N\r\n");
    chprintf(chp, "where N is time in seconds sins Unix epoch\r\n");
    chprintf(chp, "you can get current N value from unix console by the command\r\n");
    chprintf(chp, "%s", "date +\%s\r\n");
    return;
}

/*! \brief Show memory usage
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
    size_t n, size;

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: mem\r\n");
        return;
    }
    n = chHeapStatus(NULL, &size);
    chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
    chprintf(chp, "heap fragments   : %u\r\n", n);
    chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

/*! \brief Show running threads
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
    static const char *states[] = {THD_STATE_NAMES};
    Thread *tp;

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: threads\r\n");
        return;
    }
    chprintf(chp, "addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");
    tp = chRegFirstThread();
    do {
        chprintf(chp, "%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n",
                (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
                (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
        tp = chRegNextThread(tp);
    } while (tp != NULL);
}
