#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "utils_led.h"
#include "utils_shell.h"
#include "usbdetail.h"

#include "MAX2769.h"

void main(void)
{
	halInit();
	chSysInit();
	ledStart(NULL);
	max2769_init(&max2769_gps);
	const ShellCommand commands[] =
	{
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
	};
	usbSerialShellStart(commands);
#if MAX2769_SPI_DEBUG
	max2769_test_spi();
#endif
	/* Manage MAX2769 events */
	//struct EventListener ddone;
	static const evhandler_t evhndl[] =
	{
	};
	while(TRUE)
	{
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}

