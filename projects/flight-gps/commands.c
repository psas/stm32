#include "MAX2769.h"
#include "rci.h"
#include "utils_general.h"

#include "hal.h"
#include <stdlib.h>

extern const MAX2769Config max2769;
extern UARTConfig venus;

static BSEMAPHORE_DECL(txdone, 0);
void txend(UARTDriver * uart UNUSED) {
	/* Because chibios has no sane way to block on a uart transaction
	 * we do our own semaphore here
	 *
	 * One might think that checking UARTDn.txstate would be sane
	 * but the variable is modified in interrupt contexts and is not marked
	 * as volatile.
	 * */
	chBSemSignal(&txdone);
}

static void setconf(struct RCICmdData * cmd, struct RCIRetData * ret UNUSED, void * user UNUSED) {
	if(cmd->len != 8) {
		return;
	}
	char regstr[2] = {cmd->data[0], '\0'};
	int reg  = strtol(regstr, NULL, 16);
	int data = strtol(cmd->data+1, NULL, 16);

	max2769_set(reg, data);
}

static void setdebug(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret UNUSED, void * user UNUSED) {
	palTogglePad(max2769.cpld.debug.port, max2769.cpld.debug.pad);
}


static void setvenus(struct RCICmdData * cmd, struct RCIRetData * ret UNUSED, void * user UNUSED) {
	int len = cmd->len/2;
	if (len >= 100) {
		return;
	}
	uint8_t buf[100] = {};

	char temp[3] = {};
	int i;
	for(i = 0; i < cmd->len; i += 2) {
		temp[0] = cmd->data[i];
		temp[1] = cmd->data[i+1];
		temp[2] = '\0';
		buf[i/2] = strtol(temp, NULL, 16);
	}

	chBSemReset(&txdone, 0);
	uartStartSend(&UARTD6, len, buf);
	chBSemWait(&txdone);
}

const struct RCICommand RCI_CMD_CONF = {
	.name = "#CONF",
	.function = setconf,
	.user = NULL
};

const struct RCICommand RCI_CMD_DEBG = {
	.name = "#DEBG",
	.function = setdebug,
	.user = NULL
};

const struct RCICommand RCI_CMD_VNUS = {
	.name = "#VNUS",
	.function = setvenus,
	.user = NULL
};

