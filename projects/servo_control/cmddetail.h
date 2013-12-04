#ifndef _CMDDETAIL_H
#define _CMDDETAIL_H



/*
 * Includes
 * ======== ********************************************************************
 */

#include "ch.h"
#include "hal.h"


// boilerplate
#ifdef __cplusplus
extern "C" {
#endif


/*
 * Constant Definitons
 * =================== *********************************************************
 */

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)


/*
 * Declarations
 * ============ ****************************************************************
 */

void cmd_mem(BaseSequentialStream *out, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *out, int argc, char *argv[]);


// boilerplate
#ifdef __cplusplus
}
#endif
#endif
