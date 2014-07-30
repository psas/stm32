/* Knicks, knacks, and bits of code that don't really fit anywhere else.
 * Code here should not depend on any particular HAL (or other config)
 * option being enabled.
 */

#ifndef UTILS_GENERAL_H_
#define UTILS_GENERAL_H_
#include <stddef.h>
#include "ch.h"
#include "chprintf.h"

/* Used to mark things as unused so GCC will shut up about it. Useful in
 * function declarations:
 *     void foo(int bar UNUSED, void * baz UNUSED)
 */
#define UNUSED __attribute__((unused))
#define NORETURN __attribute__((noreturn))

// bah, comments
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Prefix string for debugging messages, useful in asserts
#define _STRINGIFY(line) #line
#define STRINGIFY(line) _STRINGIFY(line)
#define DBG_PREFIX __FILE__  ":" STRINGIFY(__LINE__) ", "

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

#define ARRAY_SIZE(array) sizeof(array)/sizeof((array)[0])

/* Utility for converting a struct to a network endian array and back
 * Create an array of swap structs using the SWAP_FIELD macro and pass
 * that, the structure, and the array to one of the swapped functions
 */

#define SWAP_FIELD(type, field) { offsetof(type, field), sizeof(((type *)0)->field), 1}
#define SWAP_ARRAY(type, field) { offsetof(type, field), sizeof(((type *)0)->field[0]), ARRAY_SIZE(((type *)0)->field)}

struct swap {
	size_t offset;
	size_t length;
	size_t elements;
};

void write_swapped(const struct swap *swaps, const void *data, uint8_t *buffer);
void read_swapped(const struct swap *swaps, void *data, const uint8_t *buffer);
size_t len_swapped(const struct swap *swaps);
#endif /* UTILS_GENERAL_H_ */
