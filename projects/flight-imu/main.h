/*! \file main.h
 *
 */

#ifndef _MAIN_H
#define _MAIN_H

/*! \mainpage flight-imu PSAS IMU Board
*
*
* \section intro Introduction
* 
* This is the application file for the PSAS IMU board.
* It interfaces to IMU and altimeter sensors. Data is logged
* to an on-board micro SD card and also transmitted over the
* Rocketnet (ethernet) interface.
*
* \section filestructure File Tree
*
* Directories:
* data_udp/:	Threads for transmitting and receiving messages over Rocketnet
* sdc/:	        Threads for managing sd card logging
* shell_cmd/:   Thread for debug shell
*
* Makefile      Build file
* main.c        main thread
* main.h        documentation
* mcuconf.h     ChibiOS Configuration - mcu
* chconf.h      ChibiOS Configuration - OS 
* halconf.h     ChibiOS Configuration - hardware abstraction layer
* lwipopts.h    LWiP Configuration    - Lightweight IP Stack configuration
* ffconf.h      FatFS Configuration   - For SD Card
* 
* <hr>
*
* @section notes release.notes
* release.notes
* <hr>
* @section requirements Requirements
*
* @section copyright Copyright Notes
*
*** Notes **
*
*   Some files used by the demo are not part of ChibiOS/RT but are copyright of
*   ST Microelectronics and are licensed under a different license.
*   Also note that not all the files present in the ST library are distributed
*   with ChibiOS/RT, you can find the whole library on the ST web site:
*
*                                http://www.st.com
*
* arm-psas-eabi-gcc compiler
* ChibiOS source
* Olimex stm32-e407 development board
*
* <hr>
*
*/

#endif

