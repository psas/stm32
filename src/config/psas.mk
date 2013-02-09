# Directory for PSAS configuration
PSAS_CFG=$(HOME)/Projects/stm32/src/config
%:
	mkdir -p $(OBJDIR)
	mkdir -p $(LSTDIR)
	cp $(PSAS_CFG)/olimex_stm32_e407.cfg $(BUILDDIR)
	cp $(PSAS_CFG)/oocd_flash_stm32f407.script $(BUILDDIR)




