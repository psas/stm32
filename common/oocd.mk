
OPENOCD = $(PSAS_OPENOCD)
OPENOCD_PATH = /opt/psas/openocd/bin
OPENOCD_SEARCH = $(OPENOCD_PATH)/../share/openocd/scripts
OPENOCD_HEXFILE = $(BUILDDIR)/$(PROJECT).hex

write: $(OPENOCD_HEXFILE) write_ocd

write_base:
	sudo env PATH="$(OPENOCD_PATH):$(PATH)" openocd -s $(OPENOCD) -s $(OPENOCD_SEARCH) -f $(OOCD_CFG) -c "script oocd_prep.script" -c "flash write_image erase $(OPENOCD_HEXFILE)" -c "reset" -c "shutdown"

write_ocd: OOCD_CFG = olimex_stm32_e407.cfg
write_ocd: write_base

write_stl: OOCD_CFG = stlinkv2_stm32_e407.cfg
write_stl: write_base

