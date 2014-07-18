/*! \file main.c
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp RTC experiments
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "iwdg.h"
#include "usbdetail.h"
#include "cmddetail.h"
#include "psas_rtc.h"
#include "rtc_lld.h"

/* see cmddetail.c for testing. Use 'date' in shell */

static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"date",  cmd_date},
    {NULL, NULL}
};

int main(void) {
    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    usbSerialShellStart(commands);

    iwdgStart();

    chThdSleepMilliseconds(2000);

    /* DONE: Check cal on oscilloscope on RTC_AF1. 64hz */
    psas_rtc_lld_init();

    while (TRUE) {
        chThdSleep(TIME_INFINITE);
    }
}


//! @}
