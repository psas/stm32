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
#include "usb.h"

#ifdef __cplusplus
extern "C" {
#endif

// OTG2 connector has problems on Olimex E407 board?
#define PSAS_USE_OTG1                       1

    /*
     * Serial over USB Driver structure.
     */
extern struct SerialUSBDriver SDU_PSAS;

#if PSAS_USE_OTG1
	#define USBD1_DATA_REQUEST_EP           1
	#define USBD1_DATA_AVAILABLE_EP         1
	#define USBD1_INTERRUPT_REQUEST_EP      2
#else
	#define USBD2_DATA_REQUEST_EP           1
	#define USBD2_DATA_AVAILABLE_EP         1
	#define USBD2_INTERRUPT_REQUEST_EP      2
#endif

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

