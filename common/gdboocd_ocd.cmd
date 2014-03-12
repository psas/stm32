target remote | openocd -s ../../toolchain/openocd -f olimex_stm32_e407.cfg -c "gdb_port pipe"
monitor stm32f4x.cpu configure -rtos ChibiOS
monitor reset halt
