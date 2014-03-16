#ifndef UTILS_HAL_H_
#define UTILS_HAL_H_

#include "hal.h"

struct pin {
    ioportid_t port;
    uint16_t pad;
};

void extUtilsStart(void);
void extAddCallback(const struct pin * pin, uint32_t mode, extcallback_t cb);
//FIXME: anything that wants utils_hal for the pin struct needs
//       HAL_USE_EXT defined as TRUE for extcallback_t. This should either be
//       turned on by including utils_hal or not depend on it at all.

#endif /* UTILS_HAL_H_ */
