# Directories for PSAS configuration
PSAS_COMMON        = ../../common
PSAS_OPENOCD       = ../../toolchain/openocd
PSAS_DEVICES       = $(PSAS_COMMON)/devices
PSAS_NET           = $(PSAS_COMMON)/net

%: $(OBJDIR) $(LSTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LSTDIR):
	mkdir -p $(LSTDIR)

