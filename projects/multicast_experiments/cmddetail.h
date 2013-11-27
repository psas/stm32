/*! \file cmddetail.h
 *
 */


#ifndef _CMDDETAIL_H
#define _CMDDETAIL_H

/*!
 * \addtogroup cmddetail
 * @{
 */

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif


//! @}

#endif
