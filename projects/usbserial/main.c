#include "ch.h"
#include "hal.h"

#include "usbdetail.h"
#include "utils_led.h"

/* Application entry point */
void main(void) {
	/* System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations. Config files for these
	 *   are located in halconf.h and mcuconf.h
	 */
	halInit();
	/* - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active. The kernel config file is located in chconf.h
	 */
	chSysInit();
	ledStart(NULL);
	usbSerialShellStart(NULL);
	/* main() should never return */
	while(TRUE){
		chThdSleep(TIME_INFINITE);
	}
}

