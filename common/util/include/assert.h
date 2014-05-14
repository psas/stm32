#ifndef __assert_h
#define __assert_h

#include "chdebug.h"

#if CH_DBG_ENABLE_ASSERTS

#define __assertHelp1(line) #line
#define __assertHelp2(line) __assertHelp1(line)
#define assert(e) ((void)( (!!(e)) || (chDbgPanic("assertion failure: " __FILE__ "(" __assertHelp2(__LINE__) "): " #e), 0) ))

#else

#define assert(e) ((void)0)

#endif

#endif
