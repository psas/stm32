/*
 * Handles communicating the launch detect pin status to the flight computer
 */

#ifndef _LAUNCH_DETECT_H
#define _LAUNCH_DETECT_H

#include <stdbool.h>

bool get_launch_detected(void);
void launch_detect_init(void);

#endif
