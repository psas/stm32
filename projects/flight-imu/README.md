
flight-imu PSAS IMU Board

Introduction
 
 This is the application file for the PSAS IMU board.
 It interfaces to IMU and altimeter sensors. Data is logged
 to an on-board micro SD card and also transmitted over the
 Rocketnet (ethernet) interface.

File Tree

 Directories:
 data_udp/:    Threads for transmitting and receiving messages over Rocketnet
 sdc/:         Threads for managing sd card logging
 conf/:        ChibiOS and related configuration

 Makefile      Build file
 main.c        main thread
