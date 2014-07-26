
#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "shell.h"

#include "utils_shell.h"
#include "utils_general.h"
#include "utils_led.h"

/*! The goal of this code is to run the shell through the serial terminal
 * and not the usb subsystem. Connect an FTDI serial/usb connector to the
 * appropriate (in rocketnet hub land this is pa9 and pa10)
 * These are configured as alternate functions in the board.h file.
 *
 * See the rocketnet-hub schematic for pinout.
 *
 *     stm		rnethub      ftdi
 * RX: pa10     ?            tx
 * TX: pa9      ?            rx
 *
 * In the mcuconf.h file enable the proper serial subsystem:
 * #define STM32_SERIAL_USE_USART1             TRUE
 *
 * In the halconf.h enable the serial system
 * #define HAL_USE_SERIAL                      TRUE
 */
#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

static const ShellCommand commands[] = {
	{"mem"    , cmd_mem},
	{"threads", cmd_threads},
	{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
	(BaseSequentialStream *)&SD1,
	commands
};


/*
 * Application entry point.
 */
void main(void) {
	static Thread *shelltp = NULL;
	halInit();
	chSysInit();

	ledStart(NULL);

	// start the serial port
	sdStart(&SD1, NULL);

	/*
	 * Shell manager initialization.
	 */
	shellInit();

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

