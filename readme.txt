This repo is for firmware, OS, and application code for the STM32F4xx series of

microcontrollers used on the PSAS AV3 avionics system.

To initialise the Chibios submodule run the command: git submodule update --init

Directories:

src:
    projects go here. Each ChibiOS project needs its own board files.

openocd:
    configuration files for openocd

ChibiOS:
    git submodule pulled from Github development on Chibios

toolchain:
    development director for crosscompilers
