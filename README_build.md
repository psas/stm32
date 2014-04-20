
# STM32 Firmware Development : Build notes

# Makefile Notes

## Top level Makefile - stm32/Makefile

This file contains a target 'get-deps' to install a gcc-arm toolchain to compile ARM executable code on x86 based OS machines. 

This is used for automated build testing.

### All Projects Makefile - stm32/projects/Makefile

This will build all the projects in the stm32/projects directory.

- It's a good idea to run it once before you push to verify nothing has broken.
- Projects intended for use in flight should have the prefix **flight-** Line endings should be unix
- Please don't commit binaries. 
- Please keep ChibiOS Board files in **stm32/common/boards**, projects should not have individual ones.
    - If you modify a board, create a new board file and place in **stm32/common/boards**.
- build for flight turns off all debug symbols, sets O3, all warnings are errors, disables usb, serial

#### TODO Where is this defined?
- Build for debug turns on all(?) debug symbols, sets O0, user specified warnings

#### Verbose Make Target Notes

- Skip these directories

```
SKIPDIRS = mpu flight-imu eventlogger                                                                                                                                         
```

- Create a list of all the current directories

```
SUBDIRS = $(shell find -mindepth 1 -maxdepth 1 -type d $(foreach dir,$(SKIPDIRS),-not -name "$(dir)"))
```
- Use a makefile substitution rule to create a list of files to for the *clean* target
    - [Substitution Rules](http://www.gnu.org/software/make/manual/make.html#Substitution-Refs)

```
CLEAN_SUBDIRS = $(SUBDIRS:%=install-%)
```

- This is the default target

```
all: $(SUBDIRS)
```
- Use the SUBDIRS list as targets
    - $@ is the current target
    - -C means switch to the directory listed
    - -w means print the directory to which we are changing

```
$(SUBDIRS):
        $(MAKE) -C $@ -w
```

- Targets for removing output from previous builds

```
clean: $(CLEAN_SUBDIRS)

rebuild: clean all 

$(CLEAN_SUBDIRS): 
        $(MAKE) -C $(@:install-%=%) -w clean

```

#### Individual Project Makefile - stm32/projects/*project_name*/Makefile

Each project defines specific build variables unique to that project.


These variables define the location of the common directory and a
project specific configuration directory.

```
PSAS= ../../common
CONF= conf
```

For example: a common PSAS makefile is included here

```
include $(PSAS)/psas.mk
```

## Common included Makefiles

In the stm32/common directory is a file psas.mk which is generally included in all Individual Project Makefiles
This file maintains the location of common utilities for the stm32 build system.

```
# Directories for PSAS configuration
PSAS_COMMON        = ../../common
PSAS_OPENOCD       = ../../toolchain/openocd
PSAS_DEVICES       = $(PSAS_COMMON)/devices                                                                                                                                   
PSAS_UTIL          = $(PSAS_COMMON)/util
PSAS_NET           = $(PSAS_COMMON)/net
PSAS_NETSRC        = $(PSAS_NET)/rnet_cmd_interp.c $(PSAS_NET)/net_addrs.c $(PSAS_NET)/utils_sockets.c
PSAS_BOARDS        = $(PSAS_COMMON)/boards
PSAS_RULES         = $(PSAS_OPENOCD)/openocd.mk
```



