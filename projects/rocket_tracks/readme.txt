*****************************************************************************
** ChibiOS/RT based Rocket Tracks Software                                 **
*****************************************************************************

** TARGET **

The software runs on an Olimex STM32-E407 board.

** The Software **

The application implements PID feedback control using on-chip ADC's for input
and feedback measurements. The control loop runs on a 1ms timer basis.
Functions have been added to the ChibiOS shell to allow control options and
configuration changes to be sent to the control loop.

Required inputs: Axis position must be input to an ADC
