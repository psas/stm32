/*! \file sensor_mpl.c
 */

#include "MPL3115A2.h"
#include "sensor_mpl.h"

//#define	    DEBUG_SENSOR_MPL

#ifdef	DEBUG_SENSOR_MPL
    #include "usbdetail.h"
    #include "chprintf.h"
#endif


/*!
 * \defgroup sensor_mpl Support for MPL sensor in Flight Application IMU
 * @{
 */

/*! \brief Periodic reads (post interrupt)
 *
 * Event callback.
 *
 * @param id
 */
void mpl_read_handler(eventid_t id) {
    (void) id;
    mpl3115a2_i2c_data reg;

    /* Get current data and read f_status to reset INT */
    mpl3115a2_read_P_T(mpl3115a2_driver.i2c_instance, &mpl3115a2_current_read);
    mpl3115a2_read_f_status(mpl3115a2_driver.i2c_instance);

    chEvtBroadcast(&mpl3115a2_data_event);   //! \sa data_udp_send_thread   \sa sd log thread

    /* Set up a one shot which will trigger next interrupt */
    reg =  mpl3115a2_read_ctrl_1(mpl3115a2_driver.i2c_instance);
    reg |= (1<<MPL3115A2_CTL1_OST_BIT);
    mpl3115a2_write_ctrl_1(mpl3115a2_driver.i2c_instance, reg);

#ifdef	DEBUG_SENSOR_MPL
	BaseSequentialStream *chp =  (BaseSequentialStream *)&SDU_PSAS;
	static uint16_t     count = 0;

    ++count;
    if (count > 1000) {
        chprintf(chp, "\r\n*** MPL3115A2 ***\r\n");
        chprintf(chp, "\r\n");
        chprintf(chp, "p: %d\tt: %d\r\n\r\n", mpl3115a2_current_read.mpl_pressure, mpl3115a2_current_read.mpl_temperature );
        count = 0;
    }
#endif

}

WORKING_AREA(waThread_mpl_int_1, MPL_INT_THREAD_STACKSIZE_BYTES);
/*! \brief MPL INT1 thread
 *
 */
msg_t Thread_mpl_int_1(void *arg) {
    (void)arg;
    static const evhandler_t evhndl_mplint[] = {
            mpl_read_handler
    };
    struct EventListener     evl_mplint1;

    chRegSetThreadName("mpl_int_1");

    mpl3115a2_init(mpl3115a2_driver.i2c_instance);

    chEvtRegister(&mpl3115a2_int_event,      &evl_mplint1,         0);

    while (TRUE) {
        chEvtDispatch(evhndl_mplint, chEvtWaitOneTimeout((EVENT_MASK(0)), US2ST(50)));
    }
    return -1;
}


//! @}
