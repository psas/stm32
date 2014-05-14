#include <string.h>

#include "assert.h"
#include "chdebug.h"
#include "chsys.h"

#ifndef NDEBUG

void assertionFailure(const char* message) {
  (void)message;
  chDbgPanic(message); // if !CH_DBG_ENABLED then chDbgPanic will return
  chSysHalt();
}

#endif
