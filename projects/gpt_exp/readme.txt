*****************************************************************************
** ChibiOS/RT HAL - GPT General Purpose Timer experiment
*****************************************************************************


Sun 25 August 2013 11:57:14 (PDT)
FAIL!
The Chibios GPT does not have structure to output to a GPIO. It is suggested
to use the PWM module with a 50% duty cycle and NULL callbacks....

This example now just shows how to use the GPT to generate periodic callbacks
to flash an LED.


** TARGET **

This experiment runs on the Olimex e407 board

** PURPOSE **

Understand how to use the GPT to have a timer generate a clock on an GPIO output
** Notes **

In mcuconf.h
STM32_GPT_USE_TIM3

In halconf.h
#define HAL_USE_GPT                 TRUE

In Olimex..../board.h
PIN_AFIO_AF(GPIOD_PIN14_GPT_T3, 2) |          \
    FOR GPT: GPTD3

--------------

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
