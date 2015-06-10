from bitfield import *
from collections import OrderedDict
import json

CONF1 = make_bf("CONF1", [
    ("FGAIN", c_uint, 1),
    ("FCENX", c_uint, 1),
    ("F3OR5", c_uint, 1),
    ("FBW", c_uint, 2),
    ("FCEN", c_uint, 6),
    ("ANTEN", c_uint, 1),
    ("MIXEN", c_uint, 1),
    ("LNAMODE", c_uint, 2),
    ("MIXPOLE", c_uint, 1),
    ("IMIX", c_uint, 2),
    ("ILO", c_uint, 2),
    ("ILNA2", c_uint, 2),
    ("ILNA1", c_uint, 4),
    ("IDLE", c_uint, 1),
    ("CHIPEN", c_uint, 1),
], basetype=c_uint32)

CONF2 = make_bf("CONF2", [
    ("DIEID", c_uint, 2),
    ("RESERVED1", c_uint, 1),
    ("LOEN", c_uint, 1),
    ("DRVCFG", c_uint, 2),
    ("BITS", c_uint, 3),
    ("FORMAT", c_uint, 2),
    ("AGCMODE", c_uint, 2),
    ("RESERVED2", c_uint, 2),
    ("GAINREF", c_uint, 12),
    ("IQEN", c_uint, 1),
], basetype=c_uint32)

CONF3 = make_bf("CONF3", [
    ("STRMRST", c_uint, 1),
    ("DATSYNCEN", c_uint, 1),
    ("TIMESYNCEN", c_uint, 1),
    ("STAMPEN", c_uint, 1),
    ("STRMBITS", c_uint, 2),
    ("STRMCOUNT", c_uint, 3),
    ("STRMSTOP", c_uint, 1),
    ("STRMSTART", c_uint, 1),
    ("STRMEN", c_uint, 1),
    ("PGAQEN", c_uint, 1),
    ("PGAIEN", c_uint, 1),
    ("RESERVED1", c_uint, 1),
    ("FHIPEN", c_uint, 1),
    ("FILTEN", c_uint, 1),
    ("FOFSTEN", c_uint, 1),
    ("DRVEN", c_uint, 1),
    ("ADCEN", c_uint, 1),
    ("HILOADEN", c_uint, 1),
    ("FSLOWEN", c_uint, 1),
    ("GAININ", c_uint, 6),
], basetype=c_uint32)

PLLCONF = make_bf("PLLCONF", [
    ("RESERVED1", c_uint, 2),
    ("PWRSAV", c_uint, 1),
    ("INT_PLL", c_uint, 1),
    ("CPTEST", c_uint, 3),
    ("RESERVED2", c_uint, 1),
    ("PFDEN", c_uint, 1),
    ("ICP", c_uint, 1),
    ("LDMUX", c_uint, 4),
    ("XTALCAP", c_uint, 5),
    ("IXTAL", c_uint, 2),
    ("REFDIV", c_uint, 2),
    ("RESERVED3", c_uint, 1),
    ("REFOUTEN", c_uint, 1),
    ("RESERVED4", c_uint, 1),
    ("IVCO", c_uint, 1),
    ("VCOEN", c_uint, 1),
], basetype=c_uint32)

DIV = make_bf("DIV", [
    ("RESERVED1", c_uint, 3),
    ("RDIV", c_uint, 10),
    ("NDIV", c_uint, 15),
], basetype=c_uint32)

FDIV = make_bf("FDIV", [
    ("RESERVED1", c_uint, 8),
    ("FDIV", c_uint, 20),
], basetype=c_uint32)

CLK = make_bf("CLK", [
    ("MODE", c_uint, 1),
    ("SERCLK", c_uint, 1),
    ("ADCCLK", c_uint, 1),
    ("FCLKIN", c_uint, 1),
    ("M_CNT", c_uint, 12),
    ("L_CNT", c_uint, 12),
], basetype=c_uint32)

ALL_REGISTERS = dict((
    (CONF1, 0b0000),
    (CONF2, 0b0001),
    (CONF3, 0b0010),
    (PLLCONF, 0b0011),
    (DIV, 0b0100),
    (FDIV, 0b0101),
    (CLK, 0b0111),
))

def fuzz_registers(initial, mask):
    current = initial
    while True:
        yield current
        current = (((current | ~mask) + 1) & mask) | (initial & ~mask)
        if current == initial:
            break

def join_registers(regs):
    current = 0
    for reg in regs:
        current = current | (reg.base << (32 * ALL_REGISTERS[reg.__class__]))
    return current

def split_registers(current):
    return sorted((idx, reg((current >> (32 * idx)) & 0xFFFFFFFF)) for reg, idx in ALL_REGISTERS.iteritems())

def dump_registers(regs, f):
    json.dump(OrderedDict((ALL_REGISTERS[reg.__class__], OrderedDict(reversed(list(reg.items())))) for reg in regs), f, indent=2)
