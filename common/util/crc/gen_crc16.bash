#!/bin/bash
# script to run pycrc to generate files for crc calculation

##
# More information:
# ./pycrc-0.8.1/pycrc.py -h
# https://github.com/tpircher/pycrc
# git clone https://github.com/tpircher/pycrc.git
##

# generate with in and out reflection
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=table  --algorithm=table-driven -o crc_16_reflect_table.txt &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=c-main --algorithm=table-driven -o crc_16_reflect_test.c &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=h      --algorithm=table-driven -o crc_16_reflect_test.h &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=c      --algorithm=table-driven -o crc_16_reflect.c &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=h      --algorithm=table-driven -o crc_16_reflect.h &&\
gcc -O3 crc_16_reflect_test.c -o crc_16_reflect_test

# no reflection
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=table  --reflect-in=0 --reflect-out=0 --algorithm=table-driven -o crc_16_noreflect_table.txt &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=c-main --reflect-in=0 --reflect-out=0 --algorithm=table-driven -o crc_16_noreflect_test.c &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=h      --reflect-in=0 --reflect-out=0 --algorithm=table-driven -o crc_16_noreflect_test.h &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=c      --reflect-in=0 --reflect-out=0 --algorithm=table-driven -o crc_16_noreflect.c &&\
./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --std=c99 --generate=h      --reflect-in=0 --reflect-out=0 --algorithm=table-driven -o crc_16_noreflect.h &&\
gcc -O3 crc_16_noreflect_test.c -o crc_16_noreflect_test

echo REFLECT:
./crc_16_reflect_test   -v -s hello
echo NOREFLECT:
./crc_16_noreflect_test -v -s hello



### Example run
#> ./gen_crc16.bash 
#REFLECT:
#width            = 16
#poly             = 0x8005
#reflect_in       = true
#xor_in           = 0xffff
#reflect_out      = true
#xor_out          = 0x0000
#crc_mask         = 0xffff
#msb_mask         = 0x8000
#0x34f6
#NOREFLECT:
#width            = 16
#poly             = 0x8005
#reflect_in       = false
#xor_in           = 0xffff
#reflect_out      = false
#xor_out          = 0x0000
#crc_mask         = 0xffff
#msb_mask         = 0x8000
#0x1cc5

