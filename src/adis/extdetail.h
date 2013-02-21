/*! \file extdetail.h
 *
 */

#ifndef _EXTDETAIL_H
#define _EXTDETAIL_H

#include "ch.h"
#include "hal.h"

extern const     EXTConfig       extcfg;
extern           EventSource     wkup_event;

void green_led_off(void *arg);
void extcb1(EXTDriver *extp, expchannel_t channel) ;

#endif
