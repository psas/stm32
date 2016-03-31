OPENOCD_DIR = $(PSAS_OPENOCD)
OPENOCD_HEXFILE = $(BUILDDIR)/$(PROJECT).hex
GDB_ELF = $(BUILDDIR)/$(PROJECT).elf
OOCD_CFG = stlinkv2_stm32_e407.cfg
GDB_CFG = $(PSAS_OPENOCD)/gdboocd_ocd.cmd

write: $(OPENOCD_HEXFILE) write_stl

write_base:
	openocd -s $(OPENOCD_DIR) -f $(OOCD_CFG) -c "program $(OPENOCD_HEXFILE) verify reset"

write_ocd: OOCD_CFG = olimex_stm32_e407.cfg
write_ocd: write_base

write_stl: OOCD_CFG = stlinkv2_stm32_e407.cfg
write_stl: write_base

gdb: $(GDB_ELF) gdb_stl

gdb_base:
	$(TRGT)gdb -q $(GDB_ELF) -x $(GDB_CFG)
	
gdb_ocd: GDB_CFG = $(PSAS_OPENOCD)/gdboocd_ocd.cmd
gdb_ocd: gdb_base

gdb_stl: GDB_CFG = $(PSAS_OPENOCD)/gdboocd_stl.cmd
gdb_stl: gdb_base

