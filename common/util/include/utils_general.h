/* Knicks, knacks, and bits of code that don't really fit anywhere else.
 * Code here should not depend on any particular HAL (or perhaps other config)
 * option being enabled.
 */

#ifndef UTILS_GENERAL_H_
#define UTILS_GENERAL_H_

#define UNUSED __attribute__((unused))

#define DBG_PREFIX __FILE__":"__LINE__", "__FUNCTION__" - "

#define case_chprint(chp, val)\
case val:\
    chprintf(chp, #val);\
    break;

#endif /* UTILS_GENERAL_H_ */
