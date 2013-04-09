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
#include "serial_usb.h"
#include "usb_cdc.h"
#include "usb.h"

#ifdef __cplusplus
extern "C" {
#endif
    /*
     * Serial over USB Driver structure.
     */
    extern struct SerialUSBDriver SDU1;

    /*
     * USB driver configuration.
     */
    extern const USBConfig usbcfg;

    /*
     * Serial over USB driver configuration.
     */
    extern const SerialUSBConfig serusbcfg;

    void usb_event(USBDriver *usbp, usbevent_t event) ;

    const USBDescriptor *get_descriptor(USBDriver *usbp,
            uint8_t dtype,
            uint8_t dindex,
            uint16_t lang);

#ifdef __cplusplus
}
#endif

#endif

//! @}

