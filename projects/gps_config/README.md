

# Configuration Experiments

# Development of GPS board SPI interface for MAX2769 Universal GPS Receiver

The SPI interface to the MAX2769 chip is a 3-wire or 'half-duplex' SPI implementation.
The stm32f4 claims to support this: See p801 of the STM32 reference manual (section 27.3.4)

#Configuration notes

See googledoc MAX2769 Register Configuration
##CONF1
-0b1011110011110101000110100011
-'0xbcf51a3'

##CONF2
-0b0000010101010000001010001000
-'0x550288'

##CONF3
-0b1110101011111111100000111010
-'0xeaff83a'

##PLLCONF
-0b1001101011000000000000001000
-'0x9ac0008'

##DIV
1536<13  : NDIV
16<3     : RDIV

##FDIV
Default

##STRM
Default

##CLK
L_CNT 256
M_CNT 1563
FCLKIN 0
ADCCLK 0
SERCLK 1
MODE   1   (this is the only differnt from default.

