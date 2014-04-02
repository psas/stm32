#ifndef UTILS_HAL_H_
#define UTILS_HAL_H_

#include "hal.h"

struct pin {
    ioportid_t port;
    uint16_t pad;
};

#if HAL_USE_EXT
void extUtilsStart(void);
void extAddCallback(const struct pin * pin, uint32_t mode, extcallback_t cb);
#endif

#endif /* UTILS_HAL_H_ */
