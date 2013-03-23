

Experiment with end-to-end datapath.

The datapath consists of:
    1. sensor board with stm32fxxx processor
    2. linux based flight computer
    3. control board with st32fxx processor

Two goals:
    1. functional test
    2. timing from sensor board to control board through FC.

This experiment uses the lwip stack in the ChibiOS/ext
directory..

It is developed on an Olimex stm32-e407 board

