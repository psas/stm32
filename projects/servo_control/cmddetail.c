/*! \file cmddetail.c
 *
 */

/*!
 * \defgroup cmddetail Command Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "cmddetail.h"

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

//! @}
