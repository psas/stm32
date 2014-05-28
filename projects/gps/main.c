#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "utils_led.h"
void main(void) {

    halInit();
    chSysInit();

    ledStart(NULL);

    palSetPad(GPIOD, GPIOD_EPHY_NRST);
    while(TRUE){
        chThdSleep(TIME_INFINITE);
    }

}

