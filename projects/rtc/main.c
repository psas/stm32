/*! \file main.c
 *
 * This implementation is specific to the Olimex stm32-e407 board.
 */

/*!
 * \defgroup mainapp RTC experiments
 * @{
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chrtclib.h"
#include "chprintf.h"

#include "utils_shell.h"
#include "usbdetail.h"

/*! test the rtc API
 */
void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
	static uint64_t unix_time;
	struct tm timp;

	if ((argc == 1) && (strcmp(argv[0], "get") == 0)) {
		unix_time = rtcGetTimeUnixUsec(&RTCD1);

		chprintf(chp, "%Dus - unix time\r\n", unix_time);
		rtcGetTimeTm(&RTCD1, &timp);
		chprintf(chp, "%s - formatted time string\r\n", asctime(&timp));
		return;
	}

	if ((argc == 1) && (strcmp(argv[0], "test") == 0)) {

		rtcSetTimeUnixSec(&RTCD1, 1382142229);
		chThdSleepMilliseconds(300);
		unix_time = rtcGetTimeUnixUsec(&RTCD1)/1000;
		chprintf(chp, "ms: %u\r\n", unix_time);

		unix_time = rtcGetTimeUnixUsec(&RTCD1);
		chprintf(chp, "Test:%Dus - unix time\r\n", unix_time);
		return;
	}

	if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
		unix_time = atol(argv[1]);
		if (unix_time > 0) {
			rtcSetTimeUnixSec(&RTCD1, unix_time);
			return;
		}
	}

	chprintf(chp, "Usage: date get\r\n");
	chprintf(chp, "       date test\r\n");
	chprintf(chp, "       date set N\r\n");
	chprintf(chp, "where N is time in seconds sins Unix epoch\r\n");
	chprintf(chp, "you can get current N value from unix console by the command\r\n");
	chprintf(chp, "%s", "date +\%s\r\n");
	return;
}

static const ShellCommand commands[] = {
	{"mem", cmd_mem},
	{"threads", cmd_threads},
	{"date",  cmd_date},
	{NULL, NULL}
};

int main(void) {
	halInit();
	chSysInit();

	usbSerialShellStart(commands);
	while (TRUE) {
		chThdSleep(TIME_INFINITE);
	}
}


//! @}
