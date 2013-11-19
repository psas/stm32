/*! \file extdetail.c
 *
 */

/*!
 * \defgroup extdetail EXT Utilities
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "ADIS16405.h"
#include "MPL3115A2.h"
#include "MPU9150.h"
#include "usbdetail.h"
#include "extdetail.h"

/*! \sa HAL_USE_EXT in hal_conf.h
 */
const EXTConfig extcfg = {
		{
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOD, extdetail_adis_dio1},         // D9
				{EXT_CH_MODE_DISABLED, NULL}, // 10
				{EXT_CH_MODE_DISABLED, NULL}, // 11
				{EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOF, extdetail_mpl3115a2_int_1},   // F12
				{EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOF, extdetail_mpu9150_int},       // F13
				{EXT_CH_MODE_DISABLED, NULL}, // 14 R PF3 G PF2 B PF14
				{EXT_CH_MODE_DISABLED, NULL}, // 15
				{EXT_CH_MODE_DISABLED, NULL}, //
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL},
				{EXT_CH_MODE_DISABLED, NULL}
		}
};


/*!
 * External interrupt from ADIS
 *
 * @param extp
 * @param channel
 */
void extdetail_adis_dio1(EXTDriver *extp, expchannel_t channel) {
	(void)extp;
	(void)channel;
//	BaseSequentialStream    *chp = (BaseSequentialStream *)&SDU_PSAS;
//	chprintf(chp, "\r\n***\t ADIS dio1 ***\r\n");
	chSysLockFromIsr();
	chEvtBroadcastI(&adis_dio1_event);
	chSysUnlockFromIsr();
}


/*!
 * External interrupt from MPU9150
 *
 * @param extp
 * @param channel
 */
void extdetail_mpu9150_int(EXTDriver *extp, expchannel_t channel) {
	(void)extp;
	(void)channel;

	chSysLockFromIsr();
	chEvtBroadcastI(&mpu9150_int_event);
	chSysUnlockFromIsr();
}



/*!
 * External interrupt from MPL3115A2 INT_1
 *
 * @param extp
 * @param channel
 */
void extdetail_mpl3115a2_int_1(EXTDriver *extp, expchannel_t channel) {
    (void)extp;
    (void)channel;

    chSysLockFromIsr();
    chEvtBroadcastI(&mpl3115a2_int_event);
    chSysUnlockFromIsr();
}



//! @}

