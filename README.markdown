

Mon 30 September 2013 11:36:41 (PDT)
Currently, this repo is undergoing a reorganization. We have passed
the initial 'how to get things working' phase and are moving towards a more
application focused model of development.



--------------------

This repo is for firmware, OS, and application code for the STM32F4xx series of microcontrollers used on the PSAS AV3 avionics system.

To initialise the Chibios submodule run:

    $ git submodule update --init


### Directories:

 - src
   - projects go here. Each ChibiOS project needs its own board files.
 - openocd
   - configuration files for openocd
 - ChibiOS
   - git submodule pulled from Github development on Chibios
 - toolchain
    - development director for crosscompilers
 - devices
    - specific device support files
