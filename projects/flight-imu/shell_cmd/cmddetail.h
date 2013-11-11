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
#include "chrtclib.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);


#ifdef __cplusplus
}
#endif


//! @}

#endif
