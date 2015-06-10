from registers import *
import sys

dump_registers([
    CONF1(0xA2919A3),
    CONF2(0x0550288),
    CONF3(0xEAFF1DC),
    PLLCONF(0x9EC0008),
    DIV(0x0C00080),
    FDIV(0x8000070),
    CLK(0x10061B2),
], sys.stdout)
