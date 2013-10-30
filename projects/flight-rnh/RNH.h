/*
 */

#ifndef RNH_H_
#define RNH_H_

typedef enum {RNH_PORT_1 = 1<<1, RNH_PORT_2 = 1<<2, RNH_PORT_3 = 8,
    RNH_PORT_4 = 16, RNH_PORT_6 = 64, RNH_PORT_7 = 128,
    RNH_PORT_ALL = RNH_PORT_1 | RNH_PORT_2 | RNH_PORT_3 | RNH_PORT_4 |
                   RNH_PORT_6 | RNH_PORT_7} RNH_port;

typedef enum {RNH_PORT_STATUS = 0, RNH_PORT_FAULT, RNH_PORT_ON, RNH_PORT_OFF} RNH_action;

RNH_port RNH_power(RNH_port port, RNH_action action);
#endif /* RNH_H_ */
