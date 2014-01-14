#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "cmddetail.h"


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
