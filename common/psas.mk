# Directories for PSAS configuration
PSAS_COMMON        = ../../common
PSAS_OPENOCD       = ../../toolchain/openocd
PSAS_DEVICES       = $(PSAS_COMMON)/devices
PSAS_UTIL          = $(PSAS_COMMON)/util
PSAS_NET           = $(PSAS_COMMON)/net
PSAS_NETSRC        = $(PSAS_NET)/rci.c $(PSAS_NET)/net_addrs.c $(PSAS_NET)/utils_sockets.c
PSAS_BOARDS        = $(PSAS_COMMON)/boards
PSAS_RULES         = $(PSAS_OPENOCD)/openocd.mk

flight: MAKECMDGOALS = flight
flight: all

ifeq ($(shell git diff-index --quiet HEAD . ../../common; echo $$?), 1)
INDEX_DIRTY = M
else
INDEX_DIRTY =
endif

ifeq ($(MAKECMDGOALS), )
VERSION_PREFIX = dev-
else
VERSION_PREFIX = $(MAKECMDGOALS)-
endif

ifeq ($(MAKECMDGOALS), flight)
BUILDFLAG = -DFLIGHT
endif


PSAS_VERSION = "\"$(VERSION_PREFIX)`git rev-parse --short HEAD`$(INDEX_DIRTY)\""

