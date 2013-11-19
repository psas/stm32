

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
 - common
   - code that tends to be reused in projects. At the top level it has makefiles and
     is further grouped into
   - boards
     - boardfiles for our own custom boards. Most off the shelf board boardfiles
       can be found in ChibiOS/boards
   - devices
     - for peripheral drivers
   - net
     - network IPs
   - util
     - other utilities 
 - ChibiOS
   - git submodule pulled from our own fork of ChibiOS
 - toolchain
    - development directory for cross compilers and openocd
 - notes
    - general notes that don't have a more specific location
