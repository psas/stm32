
CRC:
Most of the time implementations are looking for 'reflected' CRC 16

This code matches what the Linux Kernel implements for CRC16, a table
driven version with reflected CRC 16 ANSI (or modbus)

To select a different algorithm use the utility in the crc directory called pycrc.py
See also crc/gen_crc.bash
