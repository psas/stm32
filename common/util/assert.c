#include <string.h>

#include "assert.h"

const char*			assert_expr;
const char*			assert_file;
unsigned int		assert_line;

static BaseSequentialStream*	assertStream;

void assertionFailure(const char* expr, const char* file, unsigned int line) {
	assert_expr = expr;
	assert_file = file;
	assert_line = line;

	if (assertStream) {
		char		tmp[21], *p;
		size_t	plen;

		p = tmp + sizeof(tmp);
		*--p = 0;
		plen = 0;
		if (line != 0) {
			while (line != 0) {
				*--p = '0' + (line % 10);
				++plen;
				line /= 10;
			}
		} else {
			*--p = '0';
			++plen;
		}

		chSequentialStreamWrite(assertStream, (const uint8_t*)"Assertion failure: ", 19);
		chSequentialStreamWrite(assertStream, (const uint8_t*)file, strlen(file));
		chSequentialStreamWrite(assertStream, (const uint8_t*)" (", 2);
		chSequentialStreamWrite(assertStream, (const uint8_t*)p, plen);
		chSequentialStreamWrite(assertStream, (const uint8_t*)"): ", 3);
		chSequentialStreamWrite(assertStream, (const uint8_t*)expr, strlen(expr));
		chSequentialStreamWrite(assertStream, (const uint8_t*)"\r\n", 2);

		while (1)	// halt this way to allow assertStream to drain
			chThdSleep(TIME_INFINITE);
	} else
		chSysHalt();
}

void setAssertStream(BaseSequentialStream* s) {
	assertStream = s;
}
