from registers import *
import sys

dump_registers([
    CONF1(0xA2939A3),
    CONF2(0x8550308),
    CONF3(0xEAFE1DC),
    PLLCONF(0x9EC0008),
    DIV(0x0C00080),
    FDIV(0x8000070),
    CLK(0x10061B2),
], sys.stdout)
