/* Knicks, knacks, and bits of code that don't really fit anywhere else.
 * Code here should not depend on any particular HAL (or other config)
 * option being enabled.
 */

#ifndef UTILS_GENERAL_H_
#define UTILS_GENERAL_H_

/* Used to mark things as unused so GCC will shut up about it. Useful in
 * function declarations:
 *     void foo(int bar UNUSED, void * baz UNUSED)
 */
#define UNUSED __attribute__((unused))

// Prefix string for debugging messages, useful in asserts
#define DBG_PREFIX __FILE__":"__LINE__", "__FUNCTION__" - "

/* Makes constructing switches that print enums or flags easier:
 *     switch(flag){
 *     case_chprint(chp, FLAG_FOO)
 *     case_chprint(chp, FLAG_BAR)
 *     case_chprint(chp, FLAG_BAZ)
 *     default:
 *         chprintf(chp, "unknown");
 *     }
 */
#define case_chprint(chp, val)\
case val:\
    chprintf(chp, #val);\
    break;

#endif /* UTILS_GENERAL_H_ */
