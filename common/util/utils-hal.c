#if DEBUG_MPU9150

/* see hal/include/i2c.h */
typedef struct i2c_error_info {
    const char* err_string;
    int         error_number;
} i2c_error_info;

const char* i2c_errno_str(int32_t err) ;

#endif

#if DEBUG_MPU9150 || defined(__DOXYGEN__)
    static i2c_error_info i2c_debug_errors[] = {
            {"I2C_NO_ERROR    ",   0x00},
            {"I2C_BUS_ERROR   ",   0x01},
            {"I2C_ARBIT_LOST  ",   0x02},
            {"I2C_ACK_FAIL    ",   0x04},
            {"I2CD_ACK_FAILURE",   0x04},
            {"I2CD_OVERRUN    ",   0x08},
            {"I2CD_PEC_ERROR  ",   0x10},
            {"I2CD_TIMEOUT    ",   0x20},
            {"I2CD_SMB_ALERT  ",   0x40}
    };

    const char* i2c_errno_str(int32_t err) {
        uint8_t count_errors = sizeof(i2c_debug_errors)/sizeof(i2c_error_info);
        uint8_t i            = 0;

        for(i=0; i<count_errors; ++i) {
            if(i2c_debug_errors[i].error_number == err) {
                return i2c_debug_errors[i].err_string;
            }
        }
        return "I2C Error Unknown";
    }
#endif
