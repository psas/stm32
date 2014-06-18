#include <stdbool.h>
#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

#include "utils_general.h"
#include "utils_led.h"
#include "utils_shell.h"

/*! The goal of this code is to run the shell through the serial terminal
 * and not the usb subsystem. Connect an FTDI serial/usb connector to the
 * appropriate (in this case c6/c7 pins.)
 *
 * On the e407 the serial6 pins on portc6,7 are found on the UEXT connector.
 * See the schematic for pinout.
 *     stm      uext      ftdi
 * TX: pc6      pin3      rx (yellow)
 * RX: pc7      pin4      tx (orange)
 *
 * In the mcuconf.h file enable the proper serial subsystem:
 * #define STM32_SERIAL_USE_USART6             TRUE
 *
 * In the halconf.h enable the serial system
 * #define HAL_USE_SERIAL                      TRUE
 */
#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
void cmd_show(BaseSequentialStream *chp, int argc UNUSED, char *argv[] UNUSED) {
	chprintf(chp, "I am the E407 Board.\r\n");
}

static const ShellCommand commands[] = {
	{"show", cmd_show},
	{"mem", cmd_mem},
	{"threads", cmd_threads},
	{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
	(BaseSequentialStream *)&SD6,
	commands
};

void main(void) {

    halInit();
    chSysInit();
    ledStart(NULL);
    // start the serial port
    sdStart(&SD6, NULL);
    // Shell manager initialization.
    shellInit();

    /*
     * Normal main() thread activity, does something with the shell it looks like
     */

    static Thread *shelltp = NULL;
    while (true) {
        if (!shelltp )
            shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        else if (chThdTerminated(shelltp)) {
            chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
            shelltp = NULL;           /* Triggers spawning of a new shell.        */
        }
        chThdSleepMilliseconds(1000);
    }
}

