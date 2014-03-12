
OPENOCD = $(PSAS_OPENOCD)
OPENOCD_PATH = /opt/psas/openocd/bin
OPENOCD_SEARCH = $(OPENOCD_PATH)/../share/openocd/scripts
OPENOCD_HEXFILE = $(BUILDDIR)/$(PROJECT).hex
OOCD_CFG = olimex_stm32_e407.cfg
GDB_ELF = $(BUILDDIR)/$(PROJECT).elf
GDB_CFG = $(PSAS_COMMON)/gdboocd_ocd.cmd

write: $(OPENOCD_HEXFILE) write_ocd

write_base:
	 env PATH="$(OPENOCD_PATH):$(PATH)" openocd -s $(OPENOCD) -s $(OPENOCD_SEARCH) -f $(OOCD_CFG) -c "script oocd_prep.script" -c "flash write_image erase $(OPENOCD_HEXFILE)" -c "reset" -c "shutdown"

write_ocd: OOCD_CFG = olimex_stm32_e407.cfg
write_ocd: write_base

write_stl: OOCD_CFG = stlinkv2_stm32_e407.cfg
write_stl: write_base

gdb: $(GDB_ELF) gdb_ocd

gdb_base:
	$(TRGT)gdb -q $(GDB_ELF) -x $(GDB_CFG)
	
gdb_ocd: GDB_CFG = $(PSAS_COMMON)/gdboocd_ocd.cmd
gdb_ocd: gdb_base

gdb_stl: GDB_CFG = $(PSAS_COMMON)/gdboocd_stl.cmd
gdb_stl: gdb_base



#monitor target configure -rtos ChibiOS