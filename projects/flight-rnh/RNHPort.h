/* Utilities for managing ports on the RocketNet Hub.
 */

#ifndef RNH_PORT_H_
#define RNH_PORT_H_

#include "rnet_cmd_interp.h"

/* RNHPort is the representation of physical ports in such a way that they can
 * be treated as a bitfield. Note that:
 *  - Because of EEs the physical port number labels are off by one and the
 *    enum labels mirror that here, but not the values.
 *  - The umbilical is where port 5 would be
 *  - Port 8 is not physically connected due to a design overlook. FIXME: When
 *    this is fixed its element should be added.
 */
typedef enum {
    RNH_PORT_1 = 1<<0,
    RNH_PORT_2 = 1<<1,
    RNH_PORT_3 = 1<<2,
    RNH_PORT_4 = 1<<3,
    RNH_PORT_6 = 1<<5,
    RNH_PORT_7 = 1<<6,
    RNH_PORT_ALL = RNH_PORT_1 | RNH_PORT_2 | RNH_PORT_3 | RNH_PORT_4 |
                   RNH_PORT_6 | RNH_PORT_7
} RNHPort;

typedef void (*rnhPortFaultHandler)(RNHPort fault, void * data);

extern const struct RCICommand RCI_CMD_PORT;

/* Event that indicates when a new set of port current samples are ready to be
 * retrieved by rnhPortGetCurrentData().
 */
extern EventSource rnhPortCurrent;

/* Initializes hal subsystems needed to run ports */
void rnhPortStart(void);

/* Returns a bitfield of the on/off status of ports, masked by the port
 * parameter
 *
 * bit[n] set 1 indicates on
 * bit[n] set 0 indicates off or not selected.
 */
RNHPort rnhPortStatus(RNHPort port);

/* Returns a bitfield of the over-current fault status of ports, masked by
 * the port parameter.
 *
 * bit[n] set 1 indicates port faulting
 * bit[n] set 0 indicates not faulting or not selected
 */
RNHPort rnhPortFault(RNHPort port);

/* Sets the ports specified in port to on */
void rnhPortOn(RNHPort port);

/* Sets the ports specified in port to off */
void rnhPortOff(RNHPort port);

/* Sets a callback to handle a port fault line going high */
void rnhPortSetFaultHandler(rnhPortFaultHandler handler, void * data);

/* Copies the most recent set of port current measurements into data.
 * The event rnhPortCurrent signals when new data is ready to be retrieved
 */
void rnhPortGetCurrentData(uint16_t * data);

/* Sets the frequency that port current is sampled at */
void rnhPortSetCurrentDataRate(unsigned freq);

#endif /* RNH_PORT_H_ */
