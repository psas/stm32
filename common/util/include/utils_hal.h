#ifndef UTILS_HAL_H_
#define UTILS_HAL_H_

struct pin {
    ioportid_t port;
    uint16_t pad;
};

void extUtilsStart(void);
void extAddCallback(const struct pin * pin, uint32_t mode, extcallback_t cb);

#endif /* UTILS_HAL_H_ */
