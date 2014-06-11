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

#define VERTICAL_COMMAND_LIMIT	300
#define LATERAL_COMMAND_LIMIT	250

#define ENABLED					1
#define DISABLED				0

#define MANUAL_MODE				0
#define SIGHTLINE_MODE			1
#define NO_SHELL_MODE			2
#define FREEZE_MODE				3

#define REFMONITOR_THRESH		4000

#endif

