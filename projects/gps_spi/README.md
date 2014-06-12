
# Development of GPS board SPI interface for MAX2769 Universal GPS Receiver

The SPI interface to the MAX2769 chip is a 3-wire or 'half-duplex' SPI implementation.
The stm32f4 claims to support this: See p801 of the STM32 reference manual (section 27.3.4)

<p>

<pre>
	Configuring the SPI for half-duplex communication
	The SPI is capable of operating in half-duplex mode in 2 configurations.
		1 clock and 1 bidirectional data wire
		1 clock and 1 data wire (receive-only or transmit-only)
</pre>
