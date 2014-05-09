#ifndef __assert_h
#define __assert_h

#ifdef NDEBUG

#define setAssertStream(s) ((void)(!!(s)))
#define assert(e) ((void)(!!(e)))

#else

#include "ch.h"

void setAssertStream(BaseSequentialStream* stream);
void assertionFailure(const char* expr, const char* file, unsigned int line);

#define assert(e) ((void)( (!!(e)) || (assertionFailure(#e, __FILE__, __LINE__), 0) ))

#endif

#endif
