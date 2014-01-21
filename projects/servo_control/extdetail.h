#ifndef _EXTDETAIL_H
#define _EXTDETAIL_H


/*
 * Includes
 * ======== *******************************************************************
 */

#include <stdbool.h>

#include "ch.h"
#include "hal.h"


// boilerplate
#ifdef __cplusplus
extern "C" {
#endif


/*
 * Declarations
 * ============ ****************************************************************
 */

extern       bool        launch_detected;
extern       EventSource wakeup_event;
extern       EventSource launch_detect_event;
extern const EXTConfig   extcfg;

void extdetail_init(void);
void launch_detect_init(void);
void launch_detect_handler(eventid_t id);
void launch_detect_isr(EXTDriver *extp, expchannel_t channel);
void wakeup_button_handler(eventid_t id);
void wakeup_button_isr(EXTDriver *extp, expchannel_t channel);


// boilerplate
#ifdef __cplusplus
}
#endif
#endif
