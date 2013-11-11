/*! \file MPL3115A2.c
 *
 * Freescale Semiconductor Pressure sensor
 */

#include <stdbool.h>

#include "ch.h"
#include "hal.h"

#include "usbdetail.h"
#include "chprintf.h"
#include "extdetail.h"

#include "MPL3115A2.h"

MPL3115A2_Driver           mpl3115a2_driver;
EventSource                mpl3115a2_int_event;
EventSource                mpl3115a2_data_event;
MPL3115A2_read_data        mpl3115a2_current_read;

const       systime_t      mpl3115a2_i2c_timeout        = MS2ST(400);

const		uint8_t	       mpl3115a2_i2c_slave_addr  	= 0x60;

#if DEBUG_MPL3115A2
static void log_error(volatile char *s) {
    static        BaseSequentialStream      *chp   =  (BaseSequentialStream *)&SDU_PSAS;
    chprintf(chp, "E: %s\r\n", s);
}
#endif

#if DEBUG_MPL3115A2 || defined(__DOXYGEN__)
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

static msg_t mpl3115A2_read_register(I2CDriver* i2cptr, MPL3115A2_regaddr ra, mpl3115a2_i2c_data* d) {
    msg_t status = RDY_OK;

    mpl3115a2_driver.txbuf[0] = ra;
    i2cAcquireBus(i2cptr);
    status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, 1, mpl3115a2_i2c_timeout);
    i2cReleaseBus(i2cptr);
    if (status != RDY_OK){
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2cptr);
    }
    *d = mpl3115a2_driver.rxbuf[0];

    return status;
}

static msg_t mpl3115A2_write_register(I2CDriver* i2cptr, MPL3115A2_regaddr ra, mpl3115a2_i2c_data d) {
    msg_t status = RDY_OK;

    mpl3115a2_driver.txbuf[0] = ra;
    mpl3115a2_driver.txbuf[1] = d;
    i2cAcquireBus(i2cptr);
    status = i2cMasterTransmitTimeout(i2cptr, mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 2, mpl3115a2_driver.rxbuf, 0, mpl3115a2_i2c_timeout);
    i2cReleaseBus(i2cptr);

    return status;
}

void mpl3115a2_start(I2CDriver* i2c) {
    uint8_t         i = 0;

    mpl3115a2_driver.i2c_errors           = 0;
    mpl3115a2_driver.i2c_instance         = i2c;
    for(i=0; i<MPL3115A2_MAX_TX_BUFFER; ++i) {
        mpl3115a2_driver.txbuf[i] = 0;
    }
    for(i=0; i<MPL3115A2_MAX_RX_BUFFER; ++i) {
        mpl3115a2_driver.rxbuf[i] = 0xa5;
    }
    mpl3115a2_current_read.mpu_pressure    = 0xa5a5a5a5;
    mpl3115a2_current_read.mpu_temperature = 0xa5a5a5a5;

    chEvtInit(&mpl3115a2_int_event);
}
//
//static void mpl3115a2_reset(I2CDriver* i2c) {
//    mpl3115a2_write_ctrl_1(i2c, (1<<MPL3115A2_CTL1_RST_BIT) );
//}


mpl3115a2_i2c_data mpl3115a2_read_id(I2CDriver* i2c) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    msg_t              status = RDY_OK;
#else
    __attribute__((unused)) msg_t              status = RDY_OK;
#endif
    mpl3115a2_i2c_data rdata  = 0;
    status = mpl3115A2_read_register(i2c, MPL_WHO_AM_I, &rdata);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_WHO_AM_I (0x%x) is: 0x%x\r\n", MPL_WHO_AM_I, rdata);
    } else {
        log_error("MPL-Status bit RDY_OK\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return rdata;
}

mpl3115a2_i2c_data mpl3115a2_read_ctrl_1(I2CDriver* i2c ) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    msg_t              status = RDY_OK;
 #else
     __attribute__((unused)) msg_t              status = RDY_OK;
#endif
    mpl3115a2_i2c_data rdata  = 0;

    status = mpl3115A2_read_register(i2c, MPL_CTRL_REG1, &rdata);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_CTRL_REG1 (0x%x) is: %d\r\n", MPL_CTRL_REG1, mpl3115a2_driver.rxbuf[0]);
    } else {
        log_error("MPL-read ctrl_reg1 fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return rdata;
}

#if DEBUG_MPL3115A2
static mpl3115a2_i2c_data mpl3115a2_read_ctrl_4(I2CDriver* i2c ) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    msg_t              status = RDY_OK;
 #else
     __attribute__((unused)) msg_t              status = RDY_OK;
#endif
    mpl3115a2_i2c_data rdata  = 0;

    status = mpl3115A2_read_register(i2c, MPL_CTRL_REG4, &rdata);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_CTRL_REG4 (0x%x) is: 0x%x\r\n", MPL_CTRL_REG4, mpl3115a2_driver.rxbuf[0]);
    } else {
        log_error("MPL-read ctrl_reg4 fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return rdata;
}
#endif

/*! \brief read the 5 contiguous bytes for pressure and temperature
 *
 * @param i2c
 * @param d
 */
msg_t mpl3115a2_read_P_T(I2CDriver* i2c, MPL3115A2_read_data* d ) {
    msg_t              status;
    const int          readbytes   = 5;
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif
    mpl3115a2_driver.txbuf[0] = MPL_OUT_P_MSB;

    i2cAcquireBus(i2c);
    status = i2cMasterTransmitTimeout(i2c, mpl3115a2_i2c_slave_addr, mpl3115a2_driver.txbuf, 1, mpl3115a2_driver.rxbuf, readbytes, mpl3115a2_i2c_timeout);
    i2cReleaseBus(i2c);
    if (status != RDY_OK){
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
    }

    // out_p_mpb    out_p_csb  out_p_lsb
    // xxxx xxxx    xxxx xxxx  xxxx 0000
    //  f     f       f   f     f    0
    d->mpu_pressure    =  ((mpl3115a2_driver.rxbuf[0] << 16) | (mpl3115a2_driver.rxbuf[1] << 8) |(mpl3115a2_driver.rxbuf[2]) ) & 0xFFFFF0;

    // out_t_msb   out_t_lsb
    // xxxx xxxx   xxxx 0000
    //  f     f     f   0
    d->mpu_temperature =  (                                    (mpl3115a2_driver.rxbuf[3] << 8) |(mpl3115a2_driver.rxbuf[4]) ) & 0xFFF0 ;

#if DEBUG_MPL3115A2
    if (status != RDY_OK) {
        log_error("MPL-read P_T fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    } else {
        int i = 0;
        chprintf(chp, "\r\n");
        for(i = 0; i<readbytes; ++i) {
            chprintf(chp, "%d: 0x%x\r\n", i, mpl3115a2_driver.rxbuf[i] );
        }
    }
    chprintf(chp, "p: %d\tt: %d\r\n\r\n", d->mpu_pressure, d->mpu_temperature );
#endif

    return status;
}

#if 0   // Superseded by read_P_T
static void mpl3115a2_read_pressure(I2CDriver* i2c, MPL3115A2_read_data* d ) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif

    mpl3115a2_i2c_data rdata_msb  = 0;
    mpl3115a2_i2c_data rdata_csb  = 0;
    mpl3115a2_i2c_data rdata_lsb  = 0;

    msg_t              status = RDY_OK;

    status = mpl3115A2_read_register(i2c, MPL_OUT_P_MSB, &rdata_msb);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_OUT_P_MSB (0x%x) is: %d\r\n", MPL_OUT_P_MSB, rdata_msb);
    } else {
        log_error("MPL-out_p_msb fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    status = mpl3115A2_read_register(i2c, MPL_OUT_P_CSB, &rdata_csb);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_OUT_P_CSB (0x%x) is: %d\r\n", MPL_OUT_P_CSB, rdata_csb);
    } else {
        log_error("MPL-out_p_csb fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    status = mpl3115A2_read_register(i2c, MPL_OUT_P_LSB, &rdata_lsb);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_OUT_P_LSB (0x%x) is: %d\r\n", MPL_OUT_P_LSB, rdata_lsb);
    } else {
        log_error("MPL-out_p_lsb fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    d->mpu_pressure = ((rdata_msb<<16)|(rdata_csb<<8)|rdata_lsb) & 0xFFFFF;
}


static void mpl3115a2_read_temperature(I2CDriver* i2c, MPL3115A2_read_data* d ) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif

    mpl3115a2_i2c_data rdata_msb  = 0;
    mpl3115a2_i2c_data rdata_lsb  = 0;

    msg_t              status = RDY_OK;

    status = mpl3115A2_read_register(i2c, MPL_OUT_T_MSB, &rdata_msb);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_OUT_T_MSB (0x%x) is: %d\r\n", MPL_OUT_T_MSB, rdata_msb);
    } else {
        log_error("MPL-out_t_msb fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    status = mpl3115A2_read_register(i2c, MPL_OUT_T_LSB, &rdata_lsb);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_OUT_T_LSB (0x%x) is: %d\r\n", MPL_OUT_T_LSB, rdata_lsb);
    } else {
        log_error("MPL-out_t_lsb fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif

    d->mpu_temperature = ((rdata_msb<<8)|rdata_lsb) & 0xFFF;
}
#endif

#if 0
static mpl3115a2_i2c_data mpl3115a2_read_status(I2CDriver* i2c) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif

    mpl3115a2_i2c_data rdata  = 0;
    msg_t              status = RDY_OK;

    status = mpl3115A2_read_register(i2c, MPL_STATUS, &rdata);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_STATUS (0x%x) is: %d\r\n", MPL_STATUS, mpl3115a2_driver.rxbuf[0]);
    } else {
        log_error("MPL-Status read fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return rdata;
}
#endif

mpl3115a2_i2c_data mpl3115a2_read_f_status(I2CDriver* i2c) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    msg_t              status = RDY_OK;
 #else
     __attribute__((unused)) msg_t              status = RDY_OK;
#endif
    mpl3115a2_i2c_data rdata  = 0;

    status = mpl3115A2_read_register(i2c, MPL_F_STATUS, &rdata);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_F_STATUS (0x%x) is: %d\r\n", MPL_F_STATUS, mpl3115a2_driver.rxbuf[0]);
    } else {
        log_error("MPL-F Status read fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return rdata;
}

#if DEBUG_MPL3115A2
static mpl3115a2_i2c_data mpl3115a2_read_int_source(I2CDriver* i2c) {
#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
    msg_t              status = RDY_OK;
 #else
     __attribute__((unused)) msg_t              status = RDY_OK;
#endif
    mpl3115a2_i2c_data rdata  = 0;


    status = mpl3115A2_read_register(i2c, MPL_INT_SOURCE, &rdata);
#if DEBUG_MPL3115A2
    if (status == RDY_OK) {
        chprintf(chp, "MPL_INT_SOURCE (0x%x) is: 0x%x\r\n", MPL_INT_SOURCE, mpl3115a2_driver.rxbuf[0]);
    } else {
        log_error("MPL-MPL_INT_SOURCE read fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return rdata;
}
#endif

/*! \brief Registers can only be written in active mode
 *
 */
msg_t mpl3115a2_write_pt_data_cfg(I2CDriver* i2c, mpl3115a2_i2c_data rdata ) {

#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif
    msg_t              status = RDY_OK;

    status = mpl3115A2_write_register(i2c, MPL_PT_DATA_CFG, rdata);
#if DEBUG_MPL3115A2
    if (status != RDY_OK) {
        log_error("MPL_PT_DATA_CFG write fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %s\r\n", i2c_errno_str( mpl3115a2_driver.i2c_errors));
    }
#endif
    return status;
}

/*! \brief Registers can only be written in active mode
 *
 * @param i2c
 * @param active FALSE - STANDBY mode, true - ACTIVE
 * @param altimeter FALSE - Barometer mode, true - altimeter mode
 */
msg_t mpl3115a2_write_ctrl_1(I2CDriver* i2c, mpl3115a2_i2c_data rdata ) {

#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif
    msg_t              status = RDY_OK;

    status = mpl3115A2_write_register(i2c, MPL_CTRL_REG1, rdata);
#if DEBUG_MPL3115A2
    if (status != RDY_OK) {
        log_error("MPL-CTRL1 write fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %s\r\n", i2c_errno_str( mpl3115a2_driver.i2c_errors));
    }
#endif
    return status;
}

msg_t mpl3115a2_write_ctrl_4(I2CDriver* i2c, mpl3115a2_i2c_data rdata) {

#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif
    msg_t              status = RDY_OK;

    status = mpl3115A2_write_register(i2c, MPL_CTRL_REG4, rdata);
#if DEBUG_MPL3115A2
    if (status != RDY_OK) {
        log_error("MPL-CTRL4 write fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return status;
}

msg_t mpl3115a2_write_ctrl_5(I2CDriver* i2c, mpl3115a2_i2c_data rdata) {

#if DEBUG_MPL3115A2
    BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
#endif
    msg_t              status = RDY_OK;

    status = mpl3115A2_write_register(i2c, MPL_CTRL_REG5, rdata);
#if DEBUG_MPL3115A2
    if (status != RDY_OK) {
        log_error("MPL-CTRL5 write fail.\r\n");
        mpl3115a2_driver.i2c_errors = i2cGetErrors(i2c);
        chprintf(chp, "i2c errno: %d\r\n", mpl3115a2_driver.i2c_errors);
    }
#endif
    return status;
}

/*! \brief Initialize the registers on the pressure sensor
 *
 * @param i2c  Which I2C module on STM to use.
 */
void mpl3115a2_init(I2CDriver* i2c) {
    mpl3115a2_i2c_data          reg;

    mpl3115a2_write_ctrl_1(i2c, 0 ) ;  // put into stdby mode in order to write registers.

#if DEBUG_MPL3115A2
    /* Read id */
    mpl3115a2_read_id(i2c);
#endif
    /* Reset device */
   //    mpl3115a2_reset(i2c);
       chThdSleepMilliseconds(500);

    //  mpl3115a2_write_ctrl_1(i2c, 0 ) ;  // put into stdby mode in order to write registers.

#if DEBUG_MPL3115A2
    /* Read id */
    mpl3115a2_read_id(i2c);
#endif

    /* Clear registers  */
    mpl3115a2_read_P_T(mpl3115a2_driver.i2c_instance, &mpl3115a2_current_read);
    mpl3115a2_read_f_status(i2c);

    /* enable events */
    mpl3115a2_write_pt_data_cfg(i2c, 0b111);

    /* enable interrupt */
    mpl3115a2_write_ctrl_5(i2c, (1<<MPL3115A2_CTRL5_INT_CFG_DRDY));          // Use INT1 for DRDY
    mpl3115a2_write_ctrl_4(i2c, (1<<MPL3115A2_CTRL4_DRDY_INT_BIT) );         // Enable interrupts

    reg =  mpl3115a2_read_ctrl_1(mpl3115a2_driver.i2c_instance);             // read OST bit before writing.
    reg = (MPL_OS_2<<MPL3115A2_CTL1_OS_BITS) | (1<<MPL3115A2_CTL1_OST_BIT);  // ~10ms interrupts, one shot
    mpl3115a2_write_ctrl_1(mpl3115a2_driver.i2c_instance, reg);

#if DEBUG_MPL3115A2
    mpl3115a2_read_ctrl_4(i2c);
    mpl3115a2_read_ctrl_1(i2c);
    mpl3115a2_read_int_source(i2c);
#endif
    mpl3115a2_read_P_T(mpl3115a2_driver.i2c_instance, &mpl3115a2_current_read);
    mpl3115a2_read_f_status(i2c);

}


//! @}


