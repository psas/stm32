

Fri November 15 2013 
We are preparing to switch compilers and remove the old compiler binary from the repo. This will
make it much smaller but since history will be rewritten, all changes need to be committed before
this happens. 

--------------------

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
    - Version of OpenOCD  as of: Tue 04 March 2014 21:03:16 (PST)
        - openocd v0.7.0
    - Version of GNU cross compiler
    - How to set up toolchain
    - Programmers used for JTAG 
    - Debugging flow with GDB
        - make targets?
    - We will maintain toolchain in /usr/local not /opt or wherever,.. Sorry 



