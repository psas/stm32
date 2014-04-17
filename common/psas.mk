# Directories for PSAS configuration
PSAS_COMMON        = ../../common
PSAS_OPENOCD       = ../../toolchain/openocd
PSAS_DEVICES       = $(PSAS_COMMON)/devices
PSAS_UTIL          = $(PSAS_COMMON)/util
PSAS_NET           = $(PSAS_COMMON)/net
PSAS_NETSRC        = $(PSAS_NET)/rnet_cmd_interp.c $(PSAS_NET)/net_addrs.c $(PSAS_NET)/utils_sockets.c
PSAS_BOARDS        = $(PSAS_COMMON)/boards
PSAS_RULES         = $(PSAS_OPENOCD)/openocd.mk
