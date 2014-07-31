/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for PSAS Rocketnet Hub 1.0 board.
 */

/*
 * Board identifier.
 */
#define BOARD_PSAS_ROCKETNET_HUB_1_0
#define BOARD_NAME                  "PSAS Rocketnet Hub 1.0"

/*
 * Ethernet PHY settings.
 */

#define MII_KS8999_ID                       0x0
#define BOARD_PHY_ID                MII_KS8999_ID
#define BOARD_PHY_RESET()
#define BOARD_PHY_ALWAYS_LINKED
/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                25000000
#endif


/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   330

/*
 * MCU type as defined in the ST header file stm32f4xx.h.
 */
#define STM32F40_41xxx

/*
 * IO pins assignments.
 */
//      GPIO_PORTPIN_FUNCTION
#define GPIO_A0_ETH_MCRS              0    // AF11
#define GPIO_A1_ETH_MRXC              1    // AF11
#define GPIO_A2_ADC_BAT_IMON          2
#define GPIO_A3_ETH_MCOL              3    // AF11
#define GPIO_A4_NC                    4
#define GPIO_A5_NC                    5
#define GPIO_A6_NC                    6
#define GPIO_A7_ETH_MRXDV             7    // AF11
#define GPIO_A8_PWR_SYNC              8
#define GPIO_A9_UART_TX               9
#define GPIO_A10_UART_RX              10
#define GPIO_A11_NC                   11
#define GPIO_A12_NC                   12
#define GPIO_A13_JTAG_TMS             13
#define GPIO_A14_JTAG_TCK             14
#define GPIO_A15_JTAG_TDI             15

#define GPIO_B0_ETH_MRXD2             0     // AF11
#define GPIO_B1_ETH_MRXD3             1     // AF11
#define GPIO_B2_BOOT1                 2
#define GPIO_B3_JTAG_TDO              3
#define GPIO_B4_JTAG_TRST             4
#define GPIO_B5_NC                    5
#define GPIO_B6_I2C_SCL               6
#define GPIO_B7_I2C_SDA               7
#define GPIO_B8_ETH_MTXD3             8
#define GPIO_B9_NC                    9
#define GPIO_B10_NC                   10
#define GPIO_B11_ETH_MTXEN            11
#define GPIO_B12_ETH_MTXD0            12
#define GPIO_B13_ETH_MTXD1            13
#define GPIO_B14_NC                   14
#define GPIO_B15_NC                   15

#define GPIO_C0_ADC_NODE_1_4_IMON     0
#define GPIO_C1_ADC_NODE_5_8_IMON     1
#define GPIO_C2_ETH_MTXD2             2
#define GPIO_C3_ETH_MTXC              3
#define GPIO_C4_ETH_MRXD0             4
#define GPIO_C5_ETH_MRXD1             5
#define GPIO_C6_NC                    6
#define GPIO_C7_NC                    7
#define GPIO_C8_NC                    8
#define GPIO_C9_KSZ_25MHZ             9
#define GPIO_C10_KSZ_SCK              10
#define GPIO_C11_NC                   11
#define GPIO_C12_KSZ_MOSI             12
#define GPIO_C13_UMB_DETECT           13
#define GPIO_C14_NC                   14
#define GPIO_C15_NC                   15

#define GPIO_D0_BQ24_ACOK             0
#define GPIO_D1_NC                    1
#define GPIO_D2_N_ROCKET_READY        2
#define GPIO_D3_NC                    3
#define GPIO_D4_ETH_N_RST             4
#define GPIO_D5_NC                    5
#define GPIO_D6_NC                    6
#define GPIO_D7_IMON_A0               7
#define GPIO_D8_IMON_A1               8
#define GPIO_D9_NC                    9
#define GPIO_D10_NC                   10
#define GPIO_D11_RGB_B                11
#define GPIO_D12_RGB_G                12
#define GPIO_D13_RGB_R                13
#define GPIO_D14_KSZ_EN               14
#define GPIO_D15_NC                   15

#define GPIO_E0_NODE1_N_EN            0
#define GPIO_E1_NODE2_N_EN            1
#define GPIO_E2_NODE3_N_EN            2
#define GPIO_E3_NODE4_N_EN            3
#define GPIO_E4_NC                    4
#define GPIO_E5_NODE6_N_EN            5
#define GPIO_E6_NODE7_N_EN            6
#define GPIO_E7_NODE8_N_EN            7
#define GPIO_E8_NODE1_N_FLT           8
#define GPIO_E9_NODE2_N_FLT           9
#define GPIO_E10_NODE3_N_FLT          10
#define GPIO_E11_NODE4_N_FLT          11
#define GPIO_E12_NC                   12
#define GPIO_E13_NODE6_N_FLT          13
#define GPIO_E14_NODE7_N_FLT          14
#define GPIO_E15_NODE8_N_FLT          15

#define GPIOF_PIN0                  0
#define GPIOF_PIN1                  1
#define GPIOF_PIN2                  2
#define GPIOF_PIN3                  3
#define GPIOF_PIN4                  4
#define GPIOF_PIN5                  5
#define GPIOF_PIN6                  6
#define GPIOF_PIN7                  7
#define GPIOF_PIN8                  8
#define GPIOF_PIN9                  9
#define GPIOF_PIN10                 10
#define GPIOF_USB_FS_FAULT          11
#define GPIOF_PIN12                 12
#define GPIOF_PIN13                 13
#define GPIOF_PIN14                 14
#define GPIOF_PIN15                 15

#define GPIOG_PIN0                  0
#define GPIOG_PIN1                  1
#define GPIOG_PIN2                  2
#define GPIOG_PIN3                  3
#define GPIOG_PIN4                  4
#define GPIOG_PIN5                  5
#define GPIOG_PIN6                  6
#define GPIOG_PIN7                  7
#define GPIOG_PIN8                  8
#define GPIOG_PIN9                  9
#define GPIOG_SPI2_CS               10
#define GPIOG_ETH_RMII_TXEN         11
#define GPIOG_PIN12                 12
#define GPIOG_ETH_RMII_TXD0         13
#define GPIOG_ETH_RMII_TXD1         14
#define GPIOG_PIN15                 15

#define GPIO_H0_OSC_IN              0
#define GPIO_H1_OSC_OUT             1
#define GPIOH_PIN2                  2
#define GPIOH_PIN3                  3
#define GPIOH_PIN4                  4
#define GPIOH_PIN5                  5
#define GPIOH_PIN6                  6
#define GPIOH_PIN7                  7
#define GPIOH_PIN8                  8
#define GPIOH_PIN9                  9
#define GPIOH_PIN10                 10
#define GPIOH_PIN11                 11
#define GPIOH_PIN12                 12
#define GPIOH_PIN13                 13
#define GPIOH_PIN14                 14
#define GPIOH_PIN15                 15

#define GPIOI_PIN0                  0
#define GPIOI_PIN1                  1
#define GPIOI_PIN2                  2
#define GPIOI_PIN3                  3
#define GPIOI_PIN4                  4
#define GPIOI_PIN5                  5
#define GPIOI_PIN6                  6
#define GPIOI_PIN7                  7
#define GPIOI_PIN8                  8
#define GPIOI_PIN9                  9
#define GPIOI_PIN10                 10
#define GPIOI_PIN11                 11
#define GPIOI_PIN12                 12
#define GPIOI_PIN13                 13
#define GPIOI_PIN14                 14
#define GPIOI_PIN15                 15

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * GPIO_A setup:
 *
 * Some function are input FOR NOW Tue 13 August 2013 22:33:27 (PDT)
 *
 */
#define VAL_GPIOA_MODER             (PIN_MODE_ALTERNATE(GPIO_A0_ETH_MCRS)        |\
                                     PIN_MODE_ALTERNATE(GPIO_A1_ETH_MRXC)         |\
                                     PIN_MODE_ANALOG(GPIO_A2_ADC_BAT_IMON)    |\
                                     PIN_MODE_INPUT(GPIO_A3_ETH_MCOL)    |\
                                     PIN_MODE_INPUT(GPIO_A4_NC)               |\
                                     PIN_MODE_INPUT(GPIO_A5_NC)               |\
                                     PIN_MODE_INPUT(GPIO_A6_NC)               |\
                                     PIN_MODE_ALTERNATE(GPIO_A7_ETH_MRXDV)        |\
                                     PIN_MODE_OUTPUT(GPIO_A8_PWR_SYNC)         |\
                                     PIN_MODE_ALTERNATE(GPIO_A9_UART_TX )     |\
                                     PIN_MODE_ALTERNATE(GPIO_A10_UART_RX)     |\
                                     PIN_MODE_INPUT(GPIO_A11_NC)              |\
                                     PIN_MODE_INPUT(GPIO_A12_NC)              |\
                                     PIN_MODE_ALTERNATE(GPIO_A13_JTAG_TMS)    |\
                                     PIN_MODE_ALTERNATE(GPIO_A14_JTAG_TCK)    |\
                                     PIN_MODE_ALTERNATE(GPIO_A15_JTAG_TDI))

#define VAL_GPIOA_OTYPER           (PIN_OTYPE_PUSHPULL(GPIO_A0_ETH_MCRS    ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A1_ETH_MRXC    ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A2_ADC_BAT_IMON) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A3_ETH_MCOL    ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A4_NC          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A5_NC          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A6_NC          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A7_ETH_MRXDV   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A8_PWR_SYNC    ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A9_UART_TX     ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A10_UART_RX    ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A11_NC         ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A12_NC         ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A13_JTAG_TMS   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A14_JTAG_TCK   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_A15_JTAG_TDI ))

#define VAL_GPIOA_OSPEEDR          (PIN_OSPEED_100M(GPIO_A0_ETH_MCRS       ) |\
                                     PIN_OSPEED_100M(GPIO_A1_ETH_MRXC       ) |\
                                     PIN_OSPEED_100M(GPIO_A2_ADC_BAT_IMON   ) |\
                                     PIN_OSPEED_100M(GPIO_A3_ETH_MCOL       ) |\
                                     PIN_OSPEED_100M(GPIO_A4_NC             ) |\
                                     PIN_OSPEED_100M(GPIO_A5_NC             ) |\
                                     PIN_OSPEED_100M(GPIO_A6_NC             ) |\
                                     PIN_OSPEED_100M(GPIO_A7_ETH_MRXDV      ) |\
                                     PIN_OSPEED_100M(GPIO_A8_PWR_SYNC       ) |\
                                     PIN_OSPEED_100M(GPIO_A9_UART_TX        ) |\
                                     PIN_OSPEED_100M(GPIO_A10_UART_RX       ) |\
                                     PIN_OSPEED_100M(GPIO_A11_NC            ) |\
                                     PIN_OSPEED_100M(GPIO_A12_NC            ) |\
                                     PIN_OSPEED_100M(GPIO_A13_JTAG_TMS      ) |\
                                     PIN_OSPEED_100M(GPIO_A14_JTAG_TCK      ) |\
                                     PIN_OSPEED_100M(GPIO_A15_JTAG_TDI))

#define VAL_GPIOA_PUPDR            (PIN_PUPDR_FLOATING(GPIO_A0_ETH_MCRS       ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A1_ETH_MRXC       ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A2_ADC_BAT_IMON   ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A3_ETH_MCOL       ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A4_NC             ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A5_NC             ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A6_NC             ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A7_ETH_MRXDV      ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A8_PWR_SYNC       ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A9_UART_TX        ) |\
                                     PIN_PUPDR_PULLUP  (GPIO_A10_UART_RX       ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A11_NC            ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A12_NC            ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A13_JTAG_TMS      ) |\
                                     PIN_PUPDR_PULLDOWN(GPIO_A14_JTAG_TCK      ) |\
                                     PIN_PUPDR_FLOATING(GPIO_A15_JTAG_TDI))

#define VAL_GPIOA_ODR              (PIN_ODR_HIGH(GPIO_A0_ETH_MCRS             ) |\
                                     PIN_ODR_HIGH(GPIO_A1_ETH_MRXC             ) |\
                                     PIN_ODR_HIGH(GPIO_A2_ADC_BAT_IMON         ) |\
                                     PIN_ODR_HIGH(GPIO_A3_ETH_MCOL             ) |\
                                     PIN_ODR_HIGH(GPIO_A4_NC                   ) |\
                                     PIN_ODR_HIGH(GPIO_A5_NC                   ) |\
                                     PIN_ODR_HIGH(GPIO_A6_NC                   ) |\
                                     PIN_ODR_HIGH(GPIO_A7_ETH_MRXDV            ) |\
                                     PIN_ODR_LOW(GPIO_A8_PWR_SYNC             ) |\
                                     PIN_ODR_HIGH(GPIO_A9_UART_TX              ) |\
                                     PIN_ODR_HIGH(GPIO_A10_UART_RX             ) |\
                                     PIN_ODR_HIGH(GPIO_A11_NC                  ) |\
                                     PIN_ODR_HIGH(GPIO_A12_NC                  ) |\
                                     PIN_ODR_HIGH(GPIO_A13_JTAG_TMS            ) |\
                                     PIN_ODR_HIGH(GPIO_A14_JTAG_TCK            ) |\
                                     PIN_ODR_HIGH(GPIO_A15_JTAG_TDI))

#define VAL_GPIOA_AFRL             (PIN_AFIO_AF( GPIO_A0_ETH_MCRS        ,  11  )  |\
                                     PIN_AFIO_AF( GPIO_A1_ETH_MRXC        , 11  )  |\
                                     PIN_AFIO_AF( GPIO_A2_ADC_BAT_IMON    , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A3_ETH_MCOL        , 11  )  |\
                                     PIN_AFIO_AF( GPIO_A4_NC              , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A5_NC              , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A6_NC              , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A7_ETH_MRXDV       , 11  ))

#define VAL_GPIOA_AFRH             (PIN_AFIO_AF( GPIO_A8_PWR_SYNC        , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A9_UART_TX         , 7  )  |\
                                     PIN_AFIO_AF( GPIO_A10_UART_RX        , 7  )  |\
                                     PIN_AFIO_AF( GPIO_A11_NC             , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A12_NC             , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A13_JTAG_TMS       , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A14_JTAG_TCK       , 0  )  |\
                                     PIN_AFIO_AF( GPIO_A15_JTAG_TDI       , 0  ))

/*
 * GPIOB setup:
 *
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ALTERNATE    ( GPIO_B0_ETH_MRXD2  ) |\
                                     PIN_MODE_ALTERNATE    ( GPIO_B1_ETH_MRXD3  ) |\
                                     PIN_MODE_INPUT    ( GPIO_B2_BOOT1      ) |\
                                     PIN_MODE_ALTERNATE( GPIO_B3_JTAG_TDO   ) |\
                                     PIN_MODE_ALTERNATE( GPIO_B4_JTAG_TRST  ) |\
                                     PIN_MODE_INPUT    ( GPIO_B5_NC         ) |\
                                     PIN_MODE_ALTERNATE( GPIO_B6_I2C_SCL    ) |\
                                     PIN_MODE_ALTERNATE( GPIO_B7_I2C_SDA    ) |\
                                     PIN_MODE_ALTERNATE    ( GPIO_B8_ETH_MTXD3  ) |\
                                     PIN_MODE_INPUT    ( GPIO_B9_NC         ) |\
                                     PIN_MODE_INPUT    ( GPIO_B10_NC        ) |\
                                     PIN_MODE_ALTERNATE    ( GPIO_B11_ETH_MTXEN ) |\
                                     PIN_MODE_ALTERNATE    ( GPIO_B12_ETH_MTXD0 ) |\
                                     PIN_MODE_ALTERNATE    ( GPIO_B13_ETH_MTXD1 ) |\
                                     PIN_MODE_INPUT    ( GPIO_B14_NC        ) |\
                                     PIN_MODE_INPUT    ( GPIO_B15_NC))

#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL ( GPIO_B0_ETH_MRXD2  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B1_ETH_MRXD3  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B2_BOOT1      ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B3_JTAG_TDO   ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B4_JTAG_TRST  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B5_NC         ) |\
                                     PIN_OTYPE_OPENDRAIN( GPIO_B6_I2C_SCL    ) |\
                                     PIN_OTYPE_OPENDRAIN( GPIO_B7_I2C_SDA    ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B8_ETH_MTXD3  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B9_NC         ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B10_NC        ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B11_ETH_MTXEN ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B12_ETH_MTXD0 ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B13_ETH_MTXD1 ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B14_NC        ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIO_B15_NC))

#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_100M    ( GPIO_B0_ETH_MRXD2  ) |\
                                     PIN_OSPEED_100M    ( GPIO_B1_ETH_MRXD3  ) |\
                                     PIN_OSPEED_100M    ( GPIO_B2_BOOT1      ) |\
                                     PIN_OSPEED_100M    ( GPIO_B3_JTAG_TDO   ) |\
                                     PIN_OSPEED_100M    ( GPIO_B4_JTAG_TRST  ) |\
                                     PIN_OSPEED_100M    ( GPIO_B5_NC         ) |\
                                     PIN_OSPEED_100M    ( GPIO_B6_I2C_SCL    ) |\
                                     PIN_OSPEED_100M    ( GPIO_B7_I2C_SDA    ) |\
                                     PIN_OSPEED_100M    ( GPIO_B8_ETH_MTXD3  ) |\
                                     PIN_OSPEED_100M    ( GPIO_B9_NC         ) |\
                                     PIN_OSPEED_100M    ( GPIO_B10_NC        ) |\
                                     PIN_OSPEED_100M    ( GPIO_B11_ETH_MTXEN ) |\
                                     PIN_OSPEED_100M    ( GPIO_B12_ETH_MTXD0 ) |\
                                     PIN_OSPEED_100M    ( GPIO_B13_ETH_MTXD1 ) |\
                                     PIN_OSPEED_100M    ( GPIO_B14_NC        ) |\
                                     PIN_OSPEED_100M    ( GPIO_B15_NC))

#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING( GPIO_B0_ETH_MRXD2  ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B1_ETH_MRXD3  ) |\
                                     PIN_PUPDR_PULLDOWN( GPIO_B2_BOOT1      ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B3_JTAG_TDO   ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B4_JTAG_TRST  ) |\
                                     PIN_PUPDR_PULLDOWN( GPIO_B5_NC         ) |\
                                     PIN_PUPDR_PULLUP  ( GPIO_B6_I2C_SCL    ) |\
                                     PIN_PUPDR_PULLUP  ( GPIO_B7_I2C_SDA    ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B8_ETH_MTXD3  ) |\
                                     PIN_PUPDR_PULLDOWN( GPIO_B9_NC         ) |\
                                     PIN_PUPDR_PULLDOWN( GPIO_B10_NC        ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B11_ETH_MTXEN ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B12_ETH_MTXD0 ) |\
                                     PIN_PUPDR_FLOATING( GPIO_B13_ETH_MTXD1 ) |\
                                     PIN_PUPDR_PULLDOWN( GPIO_B14_NC        ) |\
                                     PIN_PUPDR_PULLDOWN( GPIO_B15_NC))

#define VAL_GPIOB_ODR               (PIN_ODR_HIGH      ( GPIO_B0_ETH_MRXD2  ) |\
                                     PIN_ODR_HIGH      ( GPIO_B1_ETH_MRXD3  ) |\
                                     PIN_ODR_HIGH      ( GPIO_B2_BOOT1      ) |\
                                     PIN_ODR_HIGH      ( GPIO_B3_JTAG_TDO   ) |\
                                     PIN_ODR_HIGH      ( GPIO_B4_JTAG_TRST  ) |\
                                     PIN_ODR_HIGH      ( GPIO_B5_NC         ) |\
                                     PIN_ODR_HIGH      ( GPIO_B6_I2C_SCL    ) |\
                                     PIN_ODR_HIGH      ( GPIO_B7_I2C_SDA    ) |\
                                     PIN_ODR_HIGH      ( GPIO_B8_ETH_MTXD3  ) |\
                                     PIN_ODR_HIGH      ( GPIO_B9_NC         ) |\
                                     PIN_ODR_HIGH      ( GPIO_B10_NC        ) |\
                                     PIN_ODR_HIGH      ( GPIO_B11_ETH_MTXEN ) |\
                                     PIN_ODR_HIGH      ( GPIO_B12_ETH_MTXD0 ) |\
                                     PIN_ODR_HIGH      ( GPIO_B13_ETH_MTXD1 ) |\
                                     PIN_ODR_HIGH      ( GPIO_B14_NC        ) |\
                                     PIN_ODR_HIGH      ( GPIO_B15_NC))

#define VAL_GPIOB_AFRL              (PIN_AFIO_AF       ( GPIO_B0_ETH_MRXD2 , 11  )  |\
                                     PIN_AFIO_AF       ( GPIO_B1_ETH_MRXD3 , 11  )  |\
                                     PIN_AFIO_AF       ( GPIO_B2_BOOT1     , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B3_JTAG_TDO  , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B4_JTAG_TRST , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B5_NC        , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B6_I2C_SCL   , 4  )  |\
                                     PIN_AFIO_AF       ( GPIO_B7_I2C_SDA   , 4  ))

#define VAL_GPIOB_AFRH              (PIN_AFIO_AF       ( GPIO_B8_ETH_MTXD3 , 11  )  |\
                                     PIN_AFIO_AF       ( GPIO_B9_NC        , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B10_NC       , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B11_ETH_MTXEN, 11  )  |\
                                     PIN_AFIO_AF       ( GPIO_B12_ETH_MTXD0, 11  )  |\
                                     PIN_AFIO_AF       ( GPIO_B13_ETH_MTXD1, 11  )  |\
                                     PIN_AFIO_AF       ( GPIO_B14_NC       , 0  )  |\
                                     PIN_AFIO_AF       ( GPIO_B15_NC       , 0  ))


/*
 * GPIOC setup:
 *
 */
#define VAL_GPIOC_MODER             (PIN_MODE_ANALOG(GPIO_C0_ADC_NODE_1_4_IMON  ) |\
                                     PIN_MODE_ANALOG(GPIO_C1_ADC_NODE_5_8_IMON  ) |\
                                     PIN_MODE_ALTERNATE (GPIO_C2_ETH_MTXD2          ) |\
                                     PIN_MODE_ALTERNATE (GPIO_C3_ETH_MTXC           ) |\
                                     PIN_MODE_ALTERNATE (GPIO_C4_ETH_MRXD0          ) |\
                                     PIN_MODE_ALTERNATE (GPIO_C5_ETH_MRXD1          ) |\
                                     PIN_MODE_INPUT (GPIO_C6_NC                 ) |\
                                     PIN_MODE_INPUT (GPIO_C7_NC                 ) |\
                                     PIN_MODE_INPUT (GPIO_C8_NC                 ) |\
                                     PIN_MODE_INPUT (GPIO_C9_KSZ_25MHZ          ) |\
                                     PIN_MODE_INPUT (GPIO_C10_KSZ_SCK           ) |\
                                     PIN_MODE_INPUT (GPIO_C11_NC                ) |\
                                     PIN_MODE_INPUT (GPIO_C12_KSZ_MOSI          ) |\
                                     PIN_MODE_INPUT (GPIO_C13_UMB_DETECT        ) |\
                                     PIN_MODE_INPUT (GPIO_C14_NC                ) |\
                                     PIN_MODE_INPUT (GPIO_C15_NC                ))

#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIO_C0_ADC_NODE_1_4_IMON  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C1_ADC_NODE_5_8_IMON  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C2_ETH_MTXD2          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C3_ETH_MTXC           ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C4_ETH_MRXD0          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C5_ETH_MRXD1          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C6_NC                 ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C7_NC                 ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C8_NC                 ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C9_KSZ_25MHZ          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C10_KSZ_SCK           ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C11_NC                ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C12_KSZ_MOSI          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C13_UMB_DETECT        ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C14_NC                ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_C15_NC                ))

#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_100M   (GPIO_C0_ADC_NODE_1_4_IMON  ) |\
                                     PIN_OSPEED_100M   (GPIO_C1_ADC_NODE_5_8_IMON  ) |\
                                     PIN_OSPEED_100M   (GPIO_C2_ETH_MTXD2          ) |\
                                     PIN_OSPEED_100M   (GPIO_C3_ETH_MTXC           ) |\
                                     PIN_OSPEED_100M   (GPIO_C4_ETH_MRXD0          ) |\
                                     PIN_OSPEED_100M   (GPIO_C5_ETH_MRXD1          ) |\
                                     PIN_OSPEED_100M   (GPIO_C6_NC                 ) |\
                                     PIN_OSPEED_100M   (GPIO_C7_NC                 ) |\
                                     PIN_OSPEED_100M   (GPIO_C8_NC                 ) |\
                                     PIN_OSPEED_100M   (GPIO_C9_KSZ_25MHZ          ) |\
                                     PIN_OSPEED_100M   (GPIO_C10_KSZ_SCK           ) |\
                                     PIN_OSPEED_100M   (GPIO_C11_NC                ) |\
                                     PIN_OSPEED_100M   (GPIO_C12_KSZ_MOSI          ) |\
                                     PIN_OSPEED_100M   (GPIO_C13_UMB_DETECT        ) |\
                                     PIN_OSPEED_100M   (GPIO_C14_NC                ) |\
                                     PIN_OSPEED_100M   (GPIO_C15_NC                ))

#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLDOWN  (GPIO_C0_ADC_NODE_1_4_IMON  ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C1_ADC_NODE_5_8_IMON  ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_C2_ETH_MTXD2          ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_C3_ETH_MTXC           ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_C4_ETH_MRXD0          ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_C5_ETH_MRXD1          ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C6_NC                 ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C7_NC                 ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C8_NC                 ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C9_KSZ_25MHZ          ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C10_KSZ_SCK           ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C11_NC                ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C12_KSZ_MOSI          ) |\
                                     PIN_PUPDR_PULLUP    (GPIO_C13_UMB_DETECT        ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C14_NC                ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_C15_NC                ))

#define VAL_GPIOC_ODR               (PIN_ODR_HIGH        (GPIO_C0_ADC_NODE_1_4_IMON  ) |\
                                     PIN_ODR_HIGH        (GPIO_C1_ADC_NODE_5_8_IMON  ) |\
                                     PIN_ODR_HIGH        (GPIO_C2_ETH_MTXD2          ) |\
                                     PIN_ODR_HIGH        (GPIO_C3_ETH_MTXC           ) |\
                                     PIN_ODR_HIGH        (GPIO_C4_ETH_MRXD0          ) |\
                                     PIN_ODR_HIGH        (GPIO_C5_ETH_MRXD1          ) |\
                                     PIN_ODR_HIGH        (GPIO_C6_NC                 ) |\
                                     PIN_ODR_HIGH        (GPIO_C7_NC                 ) |\
                                     PIN_ODR_HIGH        (GPIO_C8_NC                 ) |\
                                     PIN_ODR_HIGH        (GPIO_C9_KSZ_25MHZ          ) |\
                                     PIN_ODR_HIGH        (GPIO_C10_KSZ_SCK           ) |\
                                     PIN_ODR_HIGH        (GPIO_C11_NC                ) |\
                                     PIN_ODR_HIGH        (GPIO_C12_KSZ_MOSI          ) |\
                                     PIN_ODR_HIGH        (GPIO_C13_UMB_DETECT        ) |\
                                     PIN_ODR_HIGH        (GPIO_C14_NC                ) |\
                                     PIN_ODR_HIGH        (GPIO_C15_NC                ))

#define VAL_GPIOC_AFRL              (PIN_AFIO_AF         ( GPIO_C0_ADC_NODE_1_4_IMON  , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C1_ADC_NODE_5_8_IMON  , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C2_ETH_MTXD2          , 11) |\
                                     PIN_AFIO_AF         ( GPIO_C3_ETH_MTXC           , 11) |\
                                     PIN_AFIO_AF         ( GPIO_C4_ETH_MRXD0          , 11) |\
                                     PIN_AFIO_AF         ( GPIO_C5_ETH_MRXD1          , 11) |\
                                     PIN_AFIO_AF         ( GPIO_C6_NC                 , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C7_NC                 , 0))

#define VAL_GPIOC_AFRH              (PIN_AFIO_AF         ( GPIO_C8_NC                 , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C9_KSZ_25MHZ          , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C10_KSZ_SCK           , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C11_NC                , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C12_KSZ_MOSI          , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C13_UMB_DETECT        , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C14_NC                , 0) |\
                                     PIN_AFIO_AF         ( GPIO_C15_NC                , 0))

/*
 * GPIOD setup:
 *
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT     ( GPIO_D0_BQ24_ACOK      )|\
                                     PIN_MODE_INPUT     ( GPIO_D1_NC             )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D2_N_ROCKET_READY )|\
                                     PIN_MODE_INPUT     ( GPIO_D3_NC             )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D4_ETH_N_RST      )|\
                                     PIN_MODE_INPUT     ( GPIO_D5_NC             )|\
                                     PIN_MODE_INPUT     ( GPIO_D6_NC             )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D7_IMON_A0        )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D8_IMON_A1        )|\
                                     PIN_MODE_INPUT     ( GPIO_D9_NC             )|\
                                     PIN_MODE_INPUT     ( GPIO_D10_NC            )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D11_RGB_B         )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D12_RGB_G         )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D13_RGB_R         )|\
                                     PIN_MODE_OUTPUT    ( GPIO_D14_KSZ_EN        )|\
                                     PIN_MODE_INPUT     ( GPIO_D15_NC            ))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL ( GPIO_D0_BQ24_ACOK      )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D1_NC             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D2_N_ROCKET_READY )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D3_NC             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D4_ETH_N_RST      )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D5_NC             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D6_NC             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D7_IMON_A0        )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D8_IMON_A1        )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D9_NC             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D10_NC            )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D11_RGB_B         )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D12_RGB_G         )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D13_RGB_R         )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D14_KSZ_EN        )|\
                                     PIN_OTYPE_PUSHPULL ( GPIO_D15_NC            ))

#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_100M    ( GPIO_D0_BQ24_ACOK      )|\
                                     PIN_OSPEED_100M    ( GPIO_D1_NC             )|\
                                     PIN_OSPEED_100M    ( GPIO_D2_N_ROCKET_READY )|\
                                     PIN_OSPEED_100M    ( GPIO_D3_NC             )|\
                                     PIN_OSPEED_100M    ( GPIO_D4_ETH_N_RST      )|\
                                     PIN_OSPEED_100M    ( GPIO_D5_NC             )|\
                                     PIN_OSPEED_100M    ( GPIO_D6_NC             )|\
                                     PIN_OSPEED_100M    ( GPIO_D7_IMON_A0        )|\
                                     PIN_OSPEED_100M    ( GPIO_D8_IMON_A1        )|\
                                     PIN_OSPEED_100M    ( GPIO_D9_NC             )|\
                                     PIN_OSPEED_100M    ( GPIO_D10_NC            )|\
                                     PIN_OSPEED_100M    ( GPIO_D11_RGB_B         )|\
                                     PIN_OSPEED_100M    ( GPIO_D12_RGB_G         )|\
                                     PIN_OSPEED_100M    ( GPIO_D13_RGB_R         )|\
                                     PIN_OSPEED_100M    ( GPIO_D14_KSZ_EN        )|\
                                     PIN_OSPEED_100M    ( GPIO_D15_NC            ))

#define VAL_GPIOD_PUPDR             (PIN_PUPDR_FLOATING ( GPIO_D0_BQ24_ACOK      )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D1_NC             )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D2_N_ROCKET_READY )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D3_NC             )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D4_ETH_N_RST      )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D5_NC             )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D6_NC             )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D7_IMON_A0        )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D8_IMON_A1        )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D9_NC             )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D10_NC            )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D11_RGB_B         )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D12_RGB_G         )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D13_RGB_R         )|\
                                     PIN_PUPDR_FLOATING ( GPIO_D14_KSZ_EN        )|\
                                     PIN_PUPDR_PULLDOWN ( GPIO_D15_NC            ))

#define VAL_GPIOD_ODR               (PIN_ODR_HIGH       ( GPIO_D0_BQ24_ACOK      )|\
                                     PIN_ODR_HIGH       ( GPIO_D1_NC             )|\
                                     PIN_ODR_HIGH       ( GPIO_D2_N_ROCKET_READY )|\
                                     PIN_ODR_HIGH       ( GPIO_D3_NC             )|\
                                     PIN_ODR_LOW        ( GPIO_D4_ETH_N_RST      )|\
                                     PIN_ODR_HIGH       ( GPIO_D5_NC             )|\
                                     PIN_ODR_HIGH       ( GPIO_D6_NC             )|\
                                     PIN_ODR_LOW        ( GPIO_D7_IMON_A0        )|\
                                     PIN_ODR_LOW        ( GPIO_D8_IMON_A1        )|\
                                     PIN_ODR_HIGH       ( GPIO_D9_NC             )|\
                                     PIN_ODR_HIGH       ( GPIO_D10_NC            )|\
                                     PIN_ODR_HIGH       ( GPIO_D11_RGB_B         )|\
                                     PIN_ODR_HIGH       ( GPIO_D12_RGB_G         )|\
                                     PIN_ODR_HIGH       ( GPIO_D13_RGB_R         )|\
                                     PIN_ODR_LOW        ( GPIO_D14_KSZ_EN        )|\
                                     PIN_ODR_HIGH       ( GPIO_D15_NC            ))

#define VAL_GPIOD_AFRL              (PIN_AFIO_AF        ( GPIO_D0_BQ24_ACOK      , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D1_NC             , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D2_N_ROCKET_READY , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D3_NC             , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D4_ETH_N_RST      , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D5_NC             , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D6_NC             , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D7_IMON_A0        , 0))

#define VAL_GPIOD_AFRH              (PIN_AFIO_AF        ( GPIO_D8_IMON_A1        , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D9_NC             , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D10_NC            , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D11_RGB_B         , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D12_RGB_G         , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D13_RGB_R         , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D14_KSZ_EN        , 0) |\
                                     PIN_AFIO_AF        ( GPIO_D15_NC            , 0))

/*
 * GPIOE setup:
 *
 */
#define VAL_GPIOE_MODER             (PIN_MODE_OUTPUT(GPIO_E0_NODE1_N_EN   ) |\
                                     PIN_MODE_OUTPUT(GPIO_E1_NODE2_N_EN   ) |\
                                     PIN_MODE_OUTPUT(GPIO_E2_NODE3_N_EN   ) |\
                                     PIN_MODE_OUTPUT(GPIO_E3_NODE4_N_EN   ) |\
                                     PIN_MODE_INPUT (GPIO_E4_NC           ) |\
                                     PIN_MODE_OUTPUT(GPIO_E5_NODE6_N_EN   ) |\
                                     PIN_MODE_OUTPUT(GPIO_E6_NODE7_N_EN   ) |\
                                     PIN_MODE_OUTPUT(GPIO_E7_NODE8_N_EN   ) |\
                                     PIN_MODE_INPUT (GPIO_E8_NODE1_N_FLT  ) |\
                                     PIN_MODE_INPUT (GPIO_E9_NODE2_N_FLT  ) |\
                                     PIN_MODE_INPUT (GPIO_E10_NODE3_N_FLT ) |\
                                     PIN_MODE_INPUT (GPIO_E11_NODE4_N_FLT ) |\
                                     PIN_MODE_INPUT (GPIO_E12_NC          ) |\
                                     PIN_MODE_INPUT (GPIO_E13_NODE6_N_FLT ) |\
                                     PIN_MODE_INPUT (GPIO_E14_NODE7_N_FLT ) |\
                                     PIN_MODE_INPUT (GPIO_E15_NODE8_N_FLT ))

#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIO_E0_NODE1_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E1_NODE2_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E2_NODE3_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E3_NODE4_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E4_NC           ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E5_NODE6_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E6_NODE7_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E7_NODE8_N_EN   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E8_NODE1_N_FLT  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E9_NODE2_N_FLT  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E10_NODE3_N_FLT ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E11_NODE4_N_FLT ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E12_NC          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E13_NODE6_N_FLT ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E14_NODE7_N_FLT ) |\
                                     PIN_OTYPE_PUSHPULL(GPIO_E15_NODE8_N_FLT ))

#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_100M   (GPIO_E0_NODE1_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E1_NODE2_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E2_NODE3_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E3_NODE4_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E4_NC           ) |\
                                     PIN_OSPEED_100M   (GPIO_E5_NODE6_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E6_NODE7_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E7_NODE8_N_EN   ) |\
                                     PIN_OSPEED_100M   (GPIO_E8_NODE1_N_FLT  ) |\
                                     PIN_OSPEED_100M   (GPIO_E9_NODE2_N_FLT  ) |\
                                     PIN_OSPEED_100M   (GPIO_E10_NODE3_N_FLT ) |\
                                     PIN_OSPEED_100M   (GPIO_E11_NODE4_N_FLT ) |\
                                     PIN_OSPEED_100M   (GPIO_E12_NC          ) |\
                                     PIN_OSPEED_100M   (GPIO_E13_NODE6_N_FLT ) |\
                                     PIN_OSPEED_100M   (GPIO_E14_NODE7_N_FLT ) |\
                                     PIN_OSPEED_100M   (GPIO_E15_NODE8_N_FLT ))

#define VAL_GPIOE_PUPDR             (PIN_PUPDR_FLOATING  (GPIO_E0_NODE1_N_EN   ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E1_NODE2_N_EN   ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E2_NODE3_N_EN   ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E3_NODE4_N_EN   ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_E4_NC           ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E5_NODE6_N_EN   ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E6_NODE7_N_EN   ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E7_NODE8_N_EN   ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E8_NODE1_N_FLT  ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E9_NODE2_N_FLT  ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E10_NODE3_N_FLT ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E11_NODE4_N_FLT ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_E12_NC          ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E13_NODE6_N_FLT ) |\
                                     PIN_PUPDR_FLOATING  (GPIO_E14_NODE7_N_FLT ) |\
                                     PIN_PUPDR_PULLDOWN  (GPIO_E15_NODE8_N_FLT ))

#define VAL_GPIOE_ODR               (PIN_ODR_HIGH      (GPIO_E0_NODE1_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E1_NODE2_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E2_NODE3_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E3_NODE4_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E4_NC           ) |\
                                     PIN_ODR_HIGH      (GPIO_E5_NODE6_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E6_NODE7_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E7_NODE8_N_EN   ) |\
                                     PIN_ODR_HIGH      (GPIO_E8_NODE1_N_FLT  ) |\
                                     PIN_ODR_HIGH      (GPIO_E9_NODE2_N_FLT  ) |\
                                     PIN_ODR_HIGH      (GPIO_E10_NODE3_N_FLT ) |\
                                     PIN_ODR_HIGH      (GPIO_E11_NODE4_N_FLT ) |\
                                     PIN_ODR_HIGH      (GPIO_E12_NC          ) |\
                                     PIN_ODR_HIGH      (GPIO_E13_NODE6_N_FLT ) |\
                                     PIN_ODR_HIGH      (GPIO_E14_NODE7_N_FLT ) |\
                                     PIN_ODR_HIGH      (GPIO_E15_NODE8_N_FLT ))

#define VAL_GPIOE_AFRL              (PIN_AFIO_AF       (GPIO_E0_NODE1_N_EN  , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E1_NODE2_N_EN  , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E2_NODE3_N_EN  , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E3_NODE4_N_EN  , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E4_NC          , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E5_NODE6_N_EN  , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E6_NODE7_N_EN  , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E7_NODE8_N_EN  , 0 ))

#define VAL_GPIOE_AFRH              (PIN_AFIO_AF       (GPIO_E8_NODE1_N_FLT , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E9_NODE2_N_FLT , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E10_NODE3_N_FLT, 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E11_NODE4_N_FLT, 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E12_NC         , 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E13_NODE6_N_FLT, 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E14_NODE7_N_FLT, 0 ) |\
                                     PIN_AFIO_AF       (GPIO_E15_NODE8_N_FLT, 0 ))

/*
 * GPIOF setup:
 *
 */
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIOF_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOF_USB_FS_FAULT) |   \
                                     PIN_MODE_INPUT(GPIOF_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN15))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_USB_FS_FAULT) |\
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN15))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_100M(GPIOF_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOF_USB_FS_FAULT) |  \
                                     PIN_OSPEED_100M(GPIOF_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN15))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_PULLUP(GPIOF_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN10) |        \
                                     PIN_PUPDR_FLOATING(GPIOF_USB_FS_FAULT) |\
                                     PIN_PUPDR_PULLUP(GPIOF_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN15))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_USB_FS_FAULT) |     \
                                     PIN_ODR_HIGH(GPIOF_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOF_USB_FS_FAULT, 0) |   \
                                     PIN_AFIO_AF(GPIOF_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN15, 0))
/*
 * GPIOG setup:
 */
#define VAL_GPIOG_MODER             (PIN_MODE_INPUT(GPIOG_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN9) |           \
                                     PIN_MODE_OUTPUT(GPIOG_SPI2_CS) |       \
                                     PIN_MODE_ALTERNATE(GPIOG_ETH_RMII_TXEN) |\
                                     PIN_MODE_INPUT(GPIOG_PIN12) |          \
                                     PIN_MODE_ALTERNATE(GPIOG_ETH_RMII_TXD0) |\
                                     PIN_MODE_ALTERNATE(GPIOG_ETH_RMII_TXD1) |\
                                     PIN_MODE_INPUT(GPIOG_PIN15))
#define VAL_GPIOG_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOG_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SPI2_CS) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_ETH_RMII_TXEN) |\
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_ETH_RMII_TXD0) |\
                                     PIN_OTYPE_PUSHPULL(GPIOG_ETH_RMII_TXD1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN15))
#define VAL_GPIOG_OSPEEDR           (PIN_OSPEED_100M(GPIOG_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOG_SPI2_CS) |       \
                                     PIN_OSPEED_100M(GPIOG_ETH_RMII_TXEN) | \
                                     PIN_OSPEED_100M(GPIOG_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOG_ETH_RMII_TXD0) | \
                                     PIN_OSPEED_100M(GPIOG_ETH_RMII_TXD1) | \
                                     PIN_OSPEED_100M(GPIOG_PIN15))
#define VAL_GPIOG_PUPDR             (PIN_PUPDR_PULLUP(GPIOG_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN9) |         \
                                     PIN_PUPDR_FLOATING(GPIOG_SPI2_CS) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_ETH_RMII_TXEN) |\
                                     PIN_PUPDR_PULLUP(GPIOG_PIN12) |        \
                                     PIN_PUPDR_FLOATING(GPIOG_ETH_RMII_TXD0) |\
                                     PIN_PUPDR_FLOATING(GPIOG_ETH_RMII_TXD1) |\
                                     PIN_PUPDR_PULLUP(GPIOG_PIN15))
#define VAL_GPIOG_ODR               (PIN_ODR_HIGH(GPIOG_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOG_SPI2_CS) |          \
                                     PIN_ODR_HIGH(GPIOG_ETH_RMII_TXEN) |    \
                                     PIN_ODR_HIGH(GPIOG_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOG_ETH_RMII_TXD0) |    \
                                     PIN_ODR_HIGH(GPIOG_ETH_RMII_TXD1) |    \
                                     PIN_ODR_HIGH(GPIOG_PIN15))
#define VAL_GPIOG_AFRL              (PIN_AFIO_AF(GPIOG_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN7, 0))
#define VAL_GPIOG_AFRH              (PIN_AFIO_AF(GPIOG_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOG_SPI2_CS, 0) |        \
                                     PIN_AFIO_AF(GPIOG_ETH_RMII_TXEN, 11) | \
                                     PIN_AFIO_AF(GPIOG_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOG_ETH_RMII_TXD0, 11) | \
                                     PIN_AFIO_AF(GPIOG_ETH_RMII_TXD1, 11) | \
                                     PIN_AFIO_AF(GPIOG_PIN15, 0))


/*
 * GPIOH setup:
 *
 */
#define VAL_GPIOH_MODER             (PIN_MODE_INPUT(GPIO_H0_OSC_IN )      |\
                                     PIN_MODE_INPUT(GPIO_H1_OSC_OUT)     |\
                                     PIN_MODE_INPUT(GPIOH_PIN2)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN3)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN4)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN5)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN6)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN7)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN8)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN9)        |\
                                     PIN_MODE_INPUT(GPIOH_PIN10)       |\
                                     PIN_MODE_INPUT(GPIOH_PIN11)       |\
                                     PIN_MODE_INPUT(GPIOH_PIN12)       |\
                                     PIN_MODE_INPUT(GPIOH_PIN13)       |\
                                     PIN_MODE_INPUT(GPIOH_PIN14)       |\
                                     PIN_MODE_INPUT(GPIOH_PIN15))
#define VAL_GPIOH_OTYPER            (PIN_OTYPE_PUSHPULL(GPIO_H0_OSC_IN)  |\
                                     PIN_OTYPE_PUSHPULL(GPIO_H1_OSC_OUT) |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN2)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN3)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN4)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN5)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN6)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN7)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN8)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN9)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN10)   |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN11)   |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN12)   |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN13)   |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN14)   |\
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN15))
#define VAL_GPIOH_OSPEEDR           (PIN_OSPEED_100M(GPIO_H0_OSC_IN)     |\
                                     PIN_OSPEED_100M(GPIO_H1_OSC_OUT)    |\
                                     PIN_OSPEED_100M(GPIOH_PIN2)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN3)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN4)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN5)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN6)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN7)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN8)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN9)       |\
                                     PIN_OSPEED_100M(GPIOH_PIN10)      |\
                                     PIN_OSPEED_100M(GPIOH_PIN11)      |\
                                     PIN_OSPEED_100M(GPIOH_PIN12)      |\
                                     PIN_OSPEED_100M(GPIOH_PIN13)      |\
                                     PIN_OSPEED_100M(GPIOH_PIN14)      |\
                                     PIN_OSPEED_100M(GPIOH_PIN15))
#define VAL_GPIOH_PUPDR             (PIN_PUPDR_FLOATING(GPIO_H0_OSC_IN)  |\
                                     PIN_PUPDR_FLOATING(GPIO_H1_OSC_OUT) |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN2)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN3)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN4)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN5)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN6)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN7)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN8)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN9)      |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN10)     |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN11)     |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN12)     |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN13)     |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN14)     |\
                                     PIN_PUPDR_PULLUP(GPIOH_PIN15))
#define VAL_GPIOH_ODR               (PIN_ODR_HIGH(GPIO_H0_OSC_IN)        |\
                                     PIN_ODR_HIGH(GPIO_H1_OSC_OUT)       |\
                                     PIN_ODR_HIGH(GPIOH_PIN2)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN3)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN4)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN5)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN6)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN7)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN8)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN9)          |\
                                     PIN_ODR_HIGH(GPIOH_PIN10)         |\
                                     PIN_ODR_HIGH(GPIOH_PIN11)         |\
                                     PIN_ODR_HIGH(GPIOH_PIN12)         |\
                                     PIN_ODR_HIGH(GPIOH_PIN13)         |\
                                     PIN_ODR_HIGH(GPIOH_PIN14)         |\
                                     PIN_ODR_HIGH(GPIOH_PIN15))

#define VAL_GPIOH_AFRL              (PIN_AFIO_AF(GPIO_H0_OSC_IN , 0)     |\
                                     PIN_AFIO_AF(GPIO_H1_OSC_OUT, 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN2   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN3   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN4   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN5   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN6   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN7   , 0))

#define VAL_GPIOH_AFRH              (PIN_AFIO_AF(GPIOH_PIN8   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN9   , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN10  , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN11  , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN12  , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN13  , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN14  , 0)     |\
                                     PIN_AFIO_AF(GPIOH_PIN15  , 0))

/*
 * GPIOI setup:
 */
#define VAL_GPIOI_MODER             (PIN_MODE_INPUT(GPIOI_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN15))
#define VAL_GPIOI_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOI_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN15))
#define VAL_GPIOI_OSPEEDR           (PIN_OSPEED_100M(GPIOI_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN15))
#define VAL_GPIOI_PUPDR             (PIN_PUPDR_PULLUP(GPIOI_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN15))
#define VAL_GPIOI_ODR               (PIN_ODR_HIGH(GPIOI_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN15))
#define VAL_GPIOI_AFRL              (PIN_AFIO_AF(GPIOI_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN7, 0))
#define VAL_GPIOI_AFRH              (PIN_AFIO_AF(GPIOI_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN15, 0))


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
