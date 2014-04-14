/*
 */

#ifndef RNH_H_
#define RNH_H_

typedef enum {RNH_PORT_1 = 1<<0, RNH_PORT_2 = 1<<1, RNH_PORT_3 = 4,
    RNH_PORT_4 = 8, RNH_PORT_6 = 32, RNH_PORT_7 = 64,
    RNH_PORT_ALL = RNH_PORT_1 | RNH_PORT_2 | RNH_PORT_3 | RNH_PORT_4 |
                   RNH_PORT_6 | RNH_PORT_7} RNH_port;

typedef enum {RNH_PORT_STATUS = 0, RNH_PORT_FAULT = 1,
              RNH_PORT_ON = 2, RNH_PORT_OFF = 3} RNH_action;

/* Takes an action on a set of ports
 * - port: a bitfield of ports where bit 0 is port 1, ..., bit 7 is port 8.
 * - action: one of STATUS, FAULT, ON, OFF
 *   - ON turns the ports in port on
 *   - OFF turns the ports in port off
 *   - FAULT returns the faulting ports, masked by the values in port
 *   - STATUS returns the on/off status of ports, masked by the values in port
 *
 * Returns a bitfield of on ports for ON, OFF and STATUS, and
 * faulting ports for FAULT
 */
RNH_port RNH_power(RNH_port port, RNH_action action);
#endif /* RNH_H_ */
