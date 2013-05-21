
OPENOCD = ../../openocd
HEXFILE = $(BUILDDIR)/$(PROJECT).hex
OOCD_CFG = olimex_stm32_e407.cfg

write_ocd:
	openocd -s $(OPENOCD) -f $(OOCD_CFG) -c "script oocd_prep.script" -c "flash write_image erase $(HEXFILE)" -c "reset" -c "shutdown"

write_stl:
	openocd -s $(OPENOCD) -f stlinkv2_stm32_e407.cfg -c "script oocd_prep.script" -c "flash write_image erase $(HEXFILE)" -c "reset" -c "shutdown"

write: $(HEXFILE)
	openocd -s $(OPENOCD) -f $(OOCD_CFG) -c "script oocd_prep.script" -c "flash write_image erase $(HEXFILE)" -c "reset" -c "shutdown"

