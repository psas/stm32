#ifndef __assert_h
#define __assert_h

#ifdef NDEBUG

#define assert(e) ((void)0)

#else

void assertionFailure(const char* message);

#define __assertHelp1(line) #line
#define __assertHelp2(line) __assertHelp1(line)
#define assert(e) ((void)( (!!(e)) || (assertionFailure("assertion failure: " __FILE__ "(" __assertHelp2(__LINE__) "): " #e), 0) ))

#endif

#endif
