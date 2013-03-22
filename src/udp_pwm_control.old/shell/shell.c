#include "chprintf.h"
#include "shell.h"
#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
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

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
   static const char *states[] = {THD_STATE_NAMES};
   Thread *tp;

   (void)argv;
   if (argc > 0) {
      chprintf(chp, "Usage: threads\r\n");
      return;
   }
   chprintf(chp, "    addr    stack prio refs     state time\r\n");
   tp = chRegFirstThread();
   do {
      chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
      tp = chRegNextThread(tp);
   } while (tp != NULL);
}

/*
 * Challenge: add additional command line functions
 */
static const ShellCommand commands[] = {
      {"mem", cmd_mem},
      {"threads", cmd_threads},
      {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
      (BaseSequentialStream *)&SDU1,
      commands
};

