/* @file usbdetail.h
 *
 */

#ifndef _USBDETAIL_H
#define _USBDETAIL_H

/*!
 * \addtogroup usbdetail
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "serial_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* If PSAS_USE_OTG1 is TRUE then the usb serial shell will use OTG1, if FALSE
 * then OTG2 is used. The OTG2 connector may have problems on the Olimex E407
 * board?
 */
#define PSAS_USE_OTG1   TRUE

extern      struct      SerialUSBDriver         SDU_PSAS;
extern      const       USBConfig               usbcfg;
extern      const       SerialUSBConfig         serusbcfg;

/* Call usbSerialShellStart with a list of commands and will
 * handle setting up the usb, usb serial, and shell drivers, as well as
 * a thread to run them in.
 */
void usbSerialShellStart(const ShellCommand* commands);

/* Sets up and then returns a BaseSequentialStream suitable for using with
 * chprintf that writes to the USB Serial driver
 */
//FIXME: This is a horrendous name.
BaseSequentialStream * getActiveUsbSerialStream(void);

#ifdef __cplusplus
}
#endif

#endif

//! @}

