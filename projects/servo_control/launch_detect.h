#ifndef _LAUNCH_DETECT_H
#define _LAUNCH_DETECT_H


/*
 * Includes
 * ======== *******************************************************************
 */

#include <stdbool.h>


// boilerplate
#ifdef __cplusplus
extern "C" {
#endif


/*
 * Declarations
 * ============ ****************************************************************
 */

bool get_launch_detected(void);
void launch_detect_init(void);


// boilerplate
#ifdef __cplusplus
}
#endif
#endif
