/*! \file main.h
 *
 */

#ifndef _MAIN_H
#define _MAIN_H

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

#define AXIS_ADC_COUNT		4
#define GPIOA_CS_SPI		4

#define VERTICAL_COMMAND_LIMIT		300
#define LATERAL_COMMAND_LIMIT		250

#define LED_OFF						0
#define LED_ON						1
#define LED_BLINK					2

/* Total number of feedback channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   1
/* Depth of the conversion buffer, channels are sampled once each.*/
#define ADC_GRP1_BUF_DEPTH      1
/* Total number of input channels to be sampled by a single ADC operation.*/
#define ADC_GRP2_NUM_CHANNELS   2
/* Depth of the conversion buffer, input channels are sampled once each.*/
#define ADC_GRP2_BUF_DEPTH      1


#endif

