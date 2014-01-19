#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


/*! \brief Convert an ADIS 14 bit accel. value to micro-g
 *
 * @param decimal
 * @param accel reading
 * @return   TRUE if less than zero, FALSE if greater or equal to zero
 */
static bool adis_accel2ug(uint32_t* decimal, uint16_t* twos_num) {
    uint16_t ones_comp;
    bool     isnegative = false;

    //! bit 13 is 14-bit two's complement sign bit
    isnegative   = (((uint16_t)(1<<13) & *twos_num) != 0) ? true : false;

    if(isnegative) {
        ones_comp    = ~(*twos_num & (uint16_t)0x3fff) & 0x3fff;
        *decimal     = (ones_comp) + 1;
    } else {
        *decimal     = *twos_num;
    }
    *decimal     *= 3330;
    return isnegative;
}


int main() {

    uint32_t result_ug;
    uint16_t zaccel=16086 ;

    adis_accel2ug(&result_ug, &zaccel);

    printf("result: %d\n", result_ug);

    return 0;
}
