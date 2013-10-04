# Directories for PSAS configuration
PSAS_CFG           = ../config
PSAS_OPENOCD       = ../../openocd
PSAS_COMMON        = ../common
PSAS_DEV_SRC       = ../../devices
PSAS_NET_COMMON    = ../net_common
%:
	mkdir -p $(OBJDIR)
	mkdir -p $(LSTDIR)
	cp $(PSAS_OPENOCD)/olimex_stm32_e407.cfg $(BUILDDIR)
	cp $(PSAS_OPENOCD)/stlinkv2_stm32_e407.cfg $(BUILDDIR)
	cp $(PSAS_OPENOCD)/oocd_flash_stm32f407.script $(BUILDDIR)
	
