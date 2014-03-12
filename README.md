This repo is for firmware, OS, and application code for the STM32F4xx series of microcontrollers used on the PSAS AV3 avionics system.

To initialize the ChibiOS submodule run:

    $ git submodule update --init

If it's been a while since you've updated ChibiOS you'll need to re-sync it as its origin has changed:

    $ git submodule sync
    $ git submodule update

### Directories:

 - projects
   - projects go here. 
   - projects used on the rocket are prefixed by flight-[a-zA-z]
   - intro projects or tool chain test projects
       - e407-led
       - enet-udp (network test)
       - ??? e407-serial
 - common
   - code that tends to be reused in projects. At the top level it has makefiles and
     is further grouped into
   - boards
     - boardfiles for our own custom boards. Most off the shelf board boardfiles
       can be found in ChibiOS/boards
   - devices
     - for peripheral drivers
   - net
     - network ports and addresses (IPs)
     - this is common across the RocketNet
        - maintained outside of the stm32 repo
   - util
     - general programming utilities
     - other utilities 
 - ChibiOS
   - git submodule pulled from our own fork of ChibiOS
 - toolchain
    - development directory for cross compilers and openocd
 - notes
    - general notes that don't have a more specific location
    


## CHIBIOS
    - As of Tue 04 March 2014 20:56:56 (PST)
    - We are using a slightly modified version of Chibios Release 2.6.3
        - This is maintained in the PSAS Github Repository
    - It is a submodule of the stm32 git project.

## BOARDS
    - PSAS is currently using two boards
        - Olimex e407
        - PSAS Rocketnet Hub

## TOOLCHAIN SETUP
#### Debugger
[OpenOCD](http://openocd.sourceforge.net/) is the tool we use to communicate, flash, and debug the microcontroller via JTAG.
  - Version v0.7.0 as of March 2014
  - Instillation instructions:
    - The Debian and Ubuntu v0.7.0 packages now have the correct build flags so it's as simple as
      - sudo apt-get install openocd
    - For other systems, or if you are building it yourself, ensure that it's built with --enable-ft2232\_libftdi --enable-stlink
  - udev rules
    - To use OpenOCD without sudo, move 99-psas-jtag.rules in common/ to /etc/udev/rules.d/ and the run sudo udevadm control --restart.
      Alternativly some versions of OpenOCD ship with a more complete udev rule set in /usr/share/openocd/contrib/

#### Compiler
[GNU Tools for ARM Embedded Processors](https://launchpad.net/gcc-arm-embedded)
  - Version 4.8 as of March 2014
  - Instillation instructions:
    - The prefered way is to use the provided [PPA](https://launchpad.net/~terry.guo/+archive/gcc-arm-embedded)

#### Other
 - Programmers used for JTAG
   - Olimex-arm-usb-ocd and STLinkV2 are what we currently use, but any supported by OpenOCD should work
 - Debugging flow with GDB
   - make targets?
