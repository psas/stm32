# STM32 Firmware Development : Build notes

## Repo Best Practices
- Projects intended for use in flight should have the prefix **flight-** Line endings should be unix
- Please don't commit binaries. 
- Please keep ChibiOS Board files in **stm32/common/boards**, projects should not have individual ones.
    - If you modify a board, create a new board file and place in **stm32/common/boards**.

## Makefile Notes

### Top level Makefile - stm32/Makefile

This file contains a target 'get-deps' to install a gcc-arm toolchain to compile ARM executable code on x86 based OS machines.
This is used for automated build testing and probably shouldn't be used to install the compiler on your personal machine.

### All Projects Makefile - stm32/projects/Makefile

This will build all the projects in the stm32/projects directory.
- The '''SKIPDIRS''' variable is intended to temporarily exclude a project from being built. Can be set from an environment variable
- '''make all''' builds all projects
- '''make clean''' Removes the output from previous builds
- '''make rebuild''' runs '''make clean''' and then '''make all'''
- It's a good idea to run it once before you push to verify nothing has broken.

### Individual Project Makefile - stm32/projects/*/Makefile

Each project defines specific build variables unique to that project. Of particular interest are
 - CSRC: This is a list of all *.c files you need to compile a project
 - INCDIR: This is a list of directories where all *.h files needed to compile a project are
 - #include */board.mk: Sets what board the project is compiled for. Be careful about changing it
to the board you're using as values in the per-project mcuconf.h will need to be changed (usually the clock), and
pin names will have changed.

#### TODO build targets
It's intended eventually to have two build targets. Not implemented yet, currently defaults to debug
- Build for flight turns off all debug symbols, sets -Ofast, -flto, all warnings are errors, disables usb, serial
- Build for debug turns on debug symbols and ChibiOS debug options, sets -Og and -ggdb, and as many warnings as is sane.

### Common included Makefiles

#### psas.mk
In stm32/common directory the makefile psas.mk which is included in all Individual Project Makefiles
This file maintains the location of common utilities for the stm32 build system. See that file for more details.

#### openocd.mk
In stm32/toolchain/openocd the makefile openocd.mk defines rules for interacting with already compiled binaries:
 - write: writes the binary out to a board.
 - gdb: Starts a gdb session with a binary already flashed to a baord.

Both rules have the variants *_ocd and *_stl that select JTAG dongle configurations.
 - *_ocd is for the Olimex ARM-USB-OCD
 - *_stl is for the STLinkV2

Rules default to *_ocd, but this can be changed by setting OOCD_CFG and GDB_CFG.

