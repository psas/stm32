/*! \file rocket_tracks.h
 *
 */

#ifndef _ROCKET_TRACKS_H
#define _ROCKET_TRACKS_H

/*! \mainpage pwm_mod PWM Modulation experiments
*
*
* \section intro Introduction
*
* This code is for experimenting with the STM32F407 microcontroller PWM interface
* and ChibiOS.
*
* \section filestructure File Tree
* Here is the basic file tree:
* main.c           - main loop is here
* usbdetail.c      - usb configuration here
* usbdetail.h      - usb configuration header
* chconf.h         - chibios configuration header
* mcuconf.h        - microcontroller configuration header
* halconf.h        - chibios hardware abstraction layer configuration header
* boards directory - this application board configuration files for ChibiOS
* <hr>
* @section notes release.notes
* release.notes
* <hr>
* @section requirements Requirements
*
* arm-psas-eabi-gcc compiler
* ChibiOS source
* Olimex stm32-e407 development board
*
* <hr>
*
*/

#define AXIS_ADC_COUNT			4

#define LAT_AXIS				0
#define VERT_AXIS				1
#define AXIS3					2
#define AXIS4					3

#define GPIOA_CS_SPI			4

#define VERTICAL_COMMAND_LIMIT	350
#define LATERAL_COMMAND_LIMIT	250

#define ENABLED					1
#define DISABLED				0

#define MANUAL_MODE				0
#define SIGHTLINE_MODE			1
#define NO_SHELL_MODE			2
#define FREEZE_MODE				3

/* Safety-Critical Code Section - DO NOT MODIFY */

#define REFMONITOR_THRESH		4000


#define LAT_HIGH_MAX		60000
#define LAT_LOW_MAX			8000
#define VERT_HIGH_MAX		62000
#define VERT_LOW_MAX		12000

#define LAT_HIGH_POS_MAX		47500
#define LAT_LOW_POS_MAX			13900
#define VERT_HIGH_POS_MAX		53000
#define VERT_LOW_POS_MAX		17800

#define POSITION_OFFSET			1024

#define OUT_OF_RANGE_TIMEOUT	5
#define MANUAL_WATCHDOG_TIMEOUT	20000

#define COL_PIXELS				1900
#define ROW_PIXELS				1080

#define COORD_TO_LAT			1
#define COORD_TO_VERT			1

typedef uint16_t axissample_t;


#endif

