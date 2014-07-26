/* Test for nonblocking chprintf */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbdetail.h"
#include "utils_led.h"

void main(void) {
	halInit();
	chSysInit();

	BaseSequentialStream * chp = getUsbStream();

	while(TRUE){
		chprintf(chp, "hello over USB Serial from the STM32!!\r\n");
		ledToggle(&GREEN);
		chThdSleepMilliseconds(500);
	}
}

