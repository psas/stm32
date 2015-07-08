#ifndef COMMANDS_H_
#define COMMANDS_H_
#include "rci.h"
#include "hal.h"

extern const struct RCICommand RCI_CMD_CONF;
extern const struct RCICommand RCI_CMD_DEBG;
extern const struct RCICommand RCI_CMD_VNUS;

void txend(UARTDriver * d);

#endif
