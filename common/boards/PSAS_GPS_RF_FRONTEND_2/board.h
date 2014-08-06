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
#define BOARD_PSAS_GPS_RF_FRONTEND_2
#define BOARD_NAME                  "PSAS GPS RF Frontend v2"

#define  MII_KS8081_ID              0x00221560 /* KSZ8081 r1.1 datasheet p31 */
/*
 * Ethernet PHY type.
 */
#define BOARD_PHY_ID                MII_KS8081_ID
#define BOARD_PHY_RMII

void mac_phy_reset(void);
#define BOARD_PHY_RESET             mac_phy_reset

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0
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
#define GPIOA_PIN0                    0
#define GPIOA_ETH_RMII_REF_CLK        1
#define GPIOA_ETH_RMII_MDIO           2
#define GPIOA_ETH_RMII_MDINT          3
#define GPIOA_PIN4                    4
#define GPIOA_CLK_SER                 5
#define GPIOA_PIN6                    6
#define GPIOA_ETH_RMII_CRS_DV         7
#define GPIOA_MCO1_ETH_CLK            8
#define GPIOA_OTG_FS_VBUS             9
#define GPIOA_PIN10                   10
#define GPIOA_OTG_FS_DM               11
#define GPIOA_OTG_FS_DP               12
#define GPIOA_JTAG_TMS                13
#define GPIOA_JTAG_TCK                14
#define GPIOA_JTAG_TDI                15

#define GPIOB_PIN0                    0
#define GPIOB_ANTFLAG                1
#define GPIOB_LD                      2
#define GPIOB_JTAG_TDO                3
#define GPIOB_PIN4                    4
#define GPIOB_DATA_OUT                5
#define GPIOB_PIN6                    6
#define GPIOB_PIN7                    7
#define GPIOB_PIN8                    8
#define GPIOB_PIN9                    9
#define GPIOB_PIN10                    10
#define GPIOB_MAX_CFG_SCLK            10
#define GPIOB_ETH_RMII_TX_EN          11
#define GPIOB_ETH_RMII_TXD0           12
#define GPIOB_ETH_RMII_TXD1           13
#define GPIOB_PIN14                   14
#define GPIOB_MAX_CFG_MISO            14
#define GPIOB_PIN15                   15
#define GPIOB_MAX_CFG_MOSI            15

#define GPIOC_PIN0                    0
#define GPIOC_ETH_RMII_MDC            1
#define GPIOC_PIN2                    2
#define GPIOC_PIN3                    3
#define GPIOC_ETH_RMII_RXD0           4
#define GPIOC_ETH_RMII_RXD1           5
#define GPIOC_USART6_TX               6
#define GPIOC_USART6_RX               7
#define GPIOC_SD_D0                   8
#define GPIOC_SD_D1                   9
#define GPIOC_SD_D2                   10
#define GPIOC_SD_D3                   11
#define GPIOC_SD_CLK                  12
#define GPIOC_PIN13                   13
#define GPIOC_PIN14                   14
#define GPIOC_PIN15                   15

#define GPIOD_SD_VDD                  0
#define GPIOD_PIN1                    1
#define GPIOD_SD_CMD                  2
#define GPIOD_EPHY_NRST               3
#define GPIOD_PIN4                    4
#define GPIOD_PIN5                    5
#define GPIOD_PIN6                    6
#define GPIOD_PIN7                    7
#define GPIOD_LED2                    8
#define GPIOD_LED4                    9
#define GPIOD_LED5                    10
#define GPIOD_RGB_R                   11
#define GPIOD_PIN12                   12
#define GPIOD_RGB_B                   13
#define GPIOD_RGB_G                   14
#define GPIOD_PIN15                   15

#define GPIOE_PIN0                    0
#define GPIOE_PIN1                    1
#define GPIOE_PIN2                    2
#define GPIOE_PIN3                    3
#define GPIOE_PIN4                    4
#define GPIOE_PIN5                    5
#define GPIOE_PIN6                    6
#define GPIOE_TIMESYNC                7
#define GPIOE_PIN8                    8
#define GPIOE_DATASYNC                9
#define GPIOE_MAX_SHDN                10
#define GPIOE_MAX_IDLE                11
#define GPIOE_MAX_CFG_CS              12
#define GPIOE_MAX_CFG_CLK             13
#define GPIOE_MAX_CFG_MOSI            14
#define GPIOE_PIN15                   15

#define GPIOH_OSC_IN                0
#define GPIOH_OSC_OUT               1

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
 */
#define VAL_GPIOA_MODER             (PIN_MODE_INPUT(GPIOA_PIN0)               |\
                                     PIN_MODE_ALTERNATE(GPIOA_ETH_RMII_REF_CLK)|\
                                     PIN_MODE_ALTERNATE(GPIOA_ETH_RMII_MDIO)  |\
                                     PIN_MODE_INPUT(GPIOA_ETH_RMII_MDINT)     |\
                                     PIN_MODE_INPUT(GPIOA_PIN4)               |\
                                     PIN_MODE_INPUT(GPIOA_CLK_SER)            |\
                                     PIN_MODE_INPUT(GPIOA_PIN6)               |\
                                     PIN_MODE_ALTERNATE(GPIOA_ETH_RMII_CRS_DV)|\
                                     PIN_MODE_ALTERNATE(GPIOA_MCO1_ETH_CLK)      |\
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_VBUS )   |\
                                     PIN_MODE_INPUT(GPIOA_PIN10)              |\
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DM)      |\
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DP)      |\
                                     PIN_MODE_ALTERNATE(GPIOA_JTAG_TMS)       |\
                                     PIN_MODE_ALTERNATE(GPIOA_JTAG_TCK)    |\
                                     PIN_MODE_ALTERNATE(GPIOA_JTAG_TDI))

#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_PIN0 )          |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_REF_CLK)|\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_MDIO)  |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_MDINT) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN4)           |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_CLK_SER)        |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN6)           |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_CRS_DV)|\
                                     PIN_OTYPE_PUSHPULL(GPIOA_MCO1_ETH_CLK)   |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_VBUS)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN10)          |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DM)      |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DP)      |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTAG_TMS)       |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTAG_TCK)    |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTAG_TDI))

#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_100M(GPIOA_PIN0)              |\
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_REF_CLK ) |\
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_MDIO)     |\
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_MDINT)    |\
                                     PIN_OSPEED_100M(GPIOA_PIN4             ) |\
                                     PIN_OSPEED_100M(GPIOA_CLK_SER             ) |\
                                     PIN_OSPEED_100M(GPIOA_PIN6             ) |\
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_CRS_DV      ) |\
                                     PIN_OSPEED_100M(GPIOA_MCO1_ETH_CLK       ) |\
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_VBUS        ) |\
                                     PIN_OSPEED_100M(GPIOA_PIN10       ) |\
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DM            ) |\
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DP            ) |\
                                     PIN_OSPEED_100M(GPIOA_JTAG_TMS      ) |\
                                     PIN_OSPEED_100M(GPIOA_JTAG_TCK      ) |\
                                     PIN_OSPEED_100M(GPIOA_JTAG_TDI))

#define VAL_GPIOA_PUPDR             (PIN_PUPDR_PULLUP(GPIOA_PIN0       ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_REF_CLK       ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_MDIO   ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_MDINT       ) |\
                                     PIN_PUPDR_PULLUP(GPIOA_PIN4             ) |\
                                     PIN_PUPDR_PULLUP(GPIOA_CLK_SER             ) |\
                                     PIN_PUPDR_PULLUP(GPIOA_PIN6             ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_CRS_DV      ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_MCO1_ETH_CLK       ) |\
                                     PIN_PUPDR_PULLDOWN(GPIOA_OTG_FS_VBUS        ) |\
                                     PIN_PUPDR_PULLUP  (GPIOA_PIN10       ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DM            ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DP            ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_JTAG_TMS      ) |\
                                     PIN_PUPDR_PULLDOWN(GPIOA_JTAG_TCK      ) |\
                                     PIN_PUPDR_FLOATING(GPIOA_JTAG_TDI))

#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_PIN0             ) |\
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_REF_CLK             ) |\
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_MDIO         ) |\
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_MDINT             ) |\
                                     PIN_ODR_HIGH(GPIOA_PIN4                   ) |\
                                     PIN_ODR_HIGH(GPIOA_CLK_SER                   ) |\
                                     PIN_ODR_HIGH(GPIOA_PIN6                   ) |\
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_CRS_DV            ) |\
                                     PIN_ODR_HIGH(GPIOA_MCO1_ETH_CLK             ) |\
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_VBUS              ) |\
                                     PIN_ODR_HIGH(GPIOA_PIN10             ) |\
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DM                  ) |\
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DP                  ) |\
                                     PIN_ODR_HIGH(GPIOA_JTAG_TMS            ) |\
                                     PIN_ODR_HIGH(GPIOA_JTAG_TCK            ) |\
                                     PIN_ODR_HIGH(GPIOA_JTAG_TDI))

#define VAL_GPIOA_AFRL              (PIN_AFIO_AF( GPIOA_PIN0        ,  0  )  |\
                                     PIN_AFIO_AF( GPIOA_ETH_RMII_REF_CLK        , 11  )  |\
                                     PIN_AFIO_AF( GPIOA_ETH_RMII_MDIO    , 11  )  |\
                                     PIN_AFIO_AF( GPIOA_ETH_RMII_MDINT        , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_PIN4              , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_CLK_SER              , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_PIN6              , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_ETH_RMII_CRS_DV       , 11  ))

#define VAL_GPIOA_AFRH              (PIN_AFIO_AF( GPIOA_MCO1_ETH_CLK        , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_OTG_FS_VBUS         , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_PIN10        , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_OTG_FS_DM             , 10  )  |\
                                     PIN_AFIO_AF( GPIOA_OTG_FS_DP             , 10  )  |\
                                     PIN_AFIO_AF( GPIOA_JTAG_TMS       , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_JTAG_TCK       , 0  )  |\
                                     PIN_AFIO_AF( GPIOA_JTAG_TDI       , 0  ))

/*
 * GPIOB setup:
 *
 */
#define VAL_GPIOB_MODER             (PIN_MODE_INPUT    ( GPIOB_PIN0  ) |\
                                     PIN_MODE_INPUT    ( GPIOB_ANTFLAG  ) |\
                                     PIN_MODE_INPUT    ( GPIOB_LD      ) |\
                                     PIN_MODE_ALTERNATE( GPIOB_JTAG_TDO   ) |\
                                     PIN_MODE_ALTERNATE( GPIOB_PIN4  ) |\
                                     PIN_MODE_INPUT    ( GPIOB_DATA_OUT         ) |\
                                     PIN_MODE_INPUT( GPIOB_PIN6    ) |\
                                     PIN_MODE_INPUT( GPIOB_PIN7    ) |\
                                     PIN_MODE_INPUT    ( GPIOB_PIN8  ) |\
                                     PIN_MODE_INPUT    ( GPIOB_PIN9         ) |\
                                     PIN_MODE_ALTERNATE    ( GPIOB_MAX_CFG_SCLK        ) |\
                                     PIN_MODE_ALTERNATE    ( GPIOB_ETH_RMII_TX_EN ) |\
                                     PIN_MODE_ALTERNATE    ( GPIOB_ETH_RMII_TXD0 ) |\
                                     PIN_MODE_ALTERNATE    ( GPIOB_ETH_RMII_TXD1 ) |\
                                     PIN_MODE_INPUT    ( GPIOB_PIN14        ) |\
                                     PIN_MODE_INPUT    ( GPIOB_MAX_CFG_MOSI))

#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL ( GPIOB_PIN0  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_ANTFLAG  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_LD      ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_JTAG_TDO   ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_PIN4  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_DATA_OUT         ) |\
                                     PIN_OTYPE_PUSHPULL( GPIOB_PIN6    ) |\
                                     PIN_OTYPE_PUSHPULL( GPIOB_PIN7    ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_PIN8  ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_PIN9         ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_MAX_CFG_SCLK        ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_ETH_RMII_TX_EN ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_ETH_RMII_TXD0 ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_ETH_RMII_TXD1 ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_PIN14        ) |\
                                     PIN_OTYPE_PUSHPULL ( GPIOB_MAX_CFG_MOSI))

#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_100M    ( GPIOB_PIN0  ) |\
                                     PIN_OSPEED_100M    ( GPIOB_ANTFLAG  ) |\
                                     PIN_OSPEED_100M    ( GPIOB_LD      ) |\
                                     PIN_OSPEED_100M    ( GPIOB_JTAG_TDO   ) |\
                                     PIN_OSPEED_100M    ( GPIOB_PIN4  ) |\
                                     PIN_OSPEED_100M    ( GPIOB_DATA_OUT         ) |\
                                     PIN_OSPEED_100M    ( GPIOB_PIN6    ) |\
                                     PIN_OSPEED_100M    ( GPIOB_PIN7    ) |\
                                     PIN_OSPEED_100M    ( GPIOB_PIN8  ) |\
                                     PIN_OSPEED_100M    ( GPIOB_PIN9         ) |\
                                     PIN_OSPEED_100M    ( GPIOB_MAX_CFG_SCLK        ) |\
                                     PIN_OSPEED_100M    ( GPIOB_ETH_RMII_TX_EN ) |\
                                     PIN_OSPEED_100M    ( GPIOB_ETH_RMII_TXD0 ) |\
                                     PIN_OSPEED_100M    ( GPIOB_ETH_RMII_TXD1 ) |\
                                     PIN_OSPEED_100M    ( GPIOB_PIN14        ) |\
                                     PIN_OSPEED_100M    ( GPIOB_MAX_CFG_MOSI))

#define VAL_GPIOB_PUPDR             (PIN_PUPDR_PULLUP( GPIOB_PIN0  ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_ANTFLAG  ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_LD      ) |\
                                     PIN_PUPDR_FLOATING( GPIOB_JTAG_TDO   ) |\
                                     PIN_PUPDR_FLOATING( GPIOB_PIN4  ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_DATA_OUT         ) |\
                                     PIN_PUPDR_PULLUP  ( GPIOB_PIN6    ) |\
                                     PIN_PUPDR_PULLUP  ( GPIOB_PIN7    ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_PIN8  ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_PIN9         ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_MAX_CFG_SCLK        ) |\
                                     PIN_PUPDR_FLOATING( GPIOB_ETH_RMII_TX_EN ) |\
                                     PIN_PUPDR_FLOATING( GPIOB_ETH_RMII_TXD0 ) |\
                                     PIN_PUPDR_FLOATING( GPIOB_ETH_RMII_TXD1 ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_PIN14        ) |\
                                     PIN_PUPDR_PULLUP( GPIOB_MAX_CFG_MOSI))

#define VAL_GPIOB_ODR               (PIN_ODR_HIGH      ( GPIOB_PIN0  ) |\
                                     PIN_ODR_HIGH      ( GPIOB_ANTFLAG  ) |\
                                     PIN_ODR_HIGH      ( GPIOB_LD      ) |\
                                     PIN_ODR_HIGH      ( GPIOB_JTAG_TDO   ) |\
                                     PIN_ODR_HIGH      ( GPIOB_PIN4  ) |\
                                     PIN_ODR_HIGH      ( GPIOB_DATA_OUT         ) |\
                                     PIN_ODR_HIGH      ( GPIOB_PIN6    ) |\
                                     PIN_ODR_HIGH      ( GPIOB_PIN7    ) |\
                                     PIN_ODR_HIGH      ( GPIOB_PIN8  ) |\
                                     PIN_ODR_HIGH      ( GPIOB_PIN9         ) |\
                                     PIN_ODR_HIGH      ( GPIOB_MAX_CFG_SCLK   ) |\
                                     PIN_ODR_HIGH      ( GPIOB_ETH_RMII_TX_EN ) |\
                                     PIN_ODR_HIGH      ( GPIOB_ETH_RMII_TXD0 ) |\
                                     PIN_ODR_HIGH      ( GPIOB_ETH_RMII_TXD1 ) |\
                                     PIN_ODR_HIGH      ( GPIOB_PIN14        ) |\
                                     PIN_ODR_HIGH      ( GPIOB_MAX_CFG_MOSI))

#define VAL_GPIOB_AFRL              (PIN_AFIO_AF       ( GPIOB_PIN0 , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_ANTFLAG , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_LD     , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_JTAG_TDO  , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_PIN4 , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_DATA_OUT        , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_PIN6   , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_PIN7   , 0  ))

#define VAL_GPIOB_AFRH              (PIN_AFIO_AF       ( GPIOB_PIN8 , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_PIN9        , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_MAX_CFG_SCLK       , 5  )  |\
                                     PIN_AFIO_AF       ( GPIOB_ETH_RMII_TX_EN, 11  )  |\
                                     PIN_AFIO_AF       ( GPIOB_ETH_RMII_TXD0, 11  )  |\
                                     PIN_AFIO_AF       ( GPIOB_ETH_RMII_TXD1, 11  )  |\
                                     PIN_AFIO_AF       ( GPIOB_PIN14       , 0  )  |\
                                     PIN_AFIO_AF       ( GPIOB_MAX_CFG_MOSI       , 5  ))


/*
 * GPIOC setup:
 *
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0  ) |\
                                     PIN_MODE_ALTERNATE(GPIOC_ETH_RMII_MDC  ) |\
                                     PIN_MODE_INPUT (GPIOC_PIN2          ) |\
                                     PIN_MODE_INPUT (GPIOC_PIN3           ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_ETH_RMII_RXD0          ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_ETH_RMII_RXD1          ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_USART6_TX                 ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_USART6_RX                 ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_SD_D0                 ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_SD_D1          ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_SD_D2           ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_SD_D3                ) |\
                                     PIN_MODE_ALTERNATE (GPIOC_SD_CLK          ) |\
                                     PIN_MODE_INPUT (GPIOC_PIN13                ) |\
                                     PIN_MODE_INPUT (GPIOC_PIN14                ) |\
                                     PIN_MODE_INPUT (GPIOC_PIN15                ))

#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_ETH_RMII_MDC  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3           ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_ETH_RMII_RXD0          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_ETH_RMII_RXD1          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_USART6_TX                 ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_USART6_RX                 ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D0                 ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D1          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D2           ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D3                ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_CLK          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13                ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14                ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15                ))

#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_100M   (GPIOC_PIN0  ) |\
                                     PIN_OSPEED_100M   (GPIOC_ETH_RMII_MDC  ) |\
                                     PIN_OSPEED_100M   (GPIOC_PIN2          ) |\
                                     PIN_OSPEED_100M   (GPIOC_PIN3           ) |\
                                     PIN_OSPEED_100M   (GPIOC_ETH_RMII_RXD0          ) |\
                                     PIN_OSPEED_100M   (GPIOC_ETH_RMII_RXD1          ) |\
                                     PIN_OSPEED_100M   (GPIOC_USART6_TX                 ) |\
                                     PIN_OSPEED_100M   (GPIOC_USART6_RX                 ) |\
                                     PIN_OSPEED_100M   (GPIOC_SD_D0                 ) |\
                                     PIN_OSPEED_100M   (GPIOC_SD_D1          ) |\
                                     PIN_OSPEED_100M   (GPIOC_SD_D2           ) |\
                                     PIN_OSPEED_100M   (GPIOC_SD_D3                ) |\
                                     PIN_OSPEED_100M   (GPIOC_SD_CLK          ) |\
                                     PIN_OSPEED_100M   (GPIOC_PIN13                ) |\
                                     PIN_OSPEED_100M   (GPIOC_PIN14                ) |\
                                     PIN_OSPEED_100M   (GPIOC_PIN15                ))

#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP  (GPIOC_PIN0  ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_ETH_RMII_MDC  ) |\
                                     PIN_PUPDR_PULLUP  (GPIOC_PIN2          ) |\
                                     PIN_PUPDR_PULLUP  (GPIOC_PIN3           ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_ETH_RMII_RXD0          ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_ETH_RMII_RXD1          ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_USART6_TX                 ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_USART6_RX                 ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_SD_D0                 ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_SD_D1          ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_SD_D2           ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_SD_D3                ) |\
                                     PIN_PUPDR_FLOATING  (GPIOC_SD_CLK          ) |\
                                     PIN_PUPDR_PULLUP  (GPIOC_PIN13                ) |\
                                     PIN_PUPDR_PULLUP  (GPIOC_PIN14                ) |\
                                     PIN_PUPDR_PULLUP  (GPIOC_PIN15                ))

#define VAL_GPIOC_ODR               (PIN_ODR_HIGH        (GPIOC_PIN0  ) |\
                                     PIN_ODR_HIGH        (GPIOC_ETH_RMII_MDC  ) |\
                                     PIN_ODR_HIGH        (GPIOC_PIN2          ) |\
                                     PIN_ODR_HIGH        (GPIOC_PIN3           ) |\
                                     PIN_ODR_HIGH        (GPIOC_ETH_RMII_RXD0          ) |\
                                     PIN_ODR_HIGH        (GPIOC_ETH_RMII_RXD1          ) |\
                                     PIN_ODR_HIGH        (GPIOC_USART6_TX                 ) |\
                                     PIN_ODR_HIGH        (GPIOC_USART6_RX                 ) |\
                                     PIN_ODR_HIGH        (GPIOC_SD_D0                 ) |\
                                     PIN_ODR_HIGH        (GPIOC_SD_D1          ) |\
                                     PIN_ODR_HIGH        (GPIOC_SD_D2           ) |\
                                     PIN_ODR_HIGH        (GPIOC_SD_D3                ) |\
                                     PIN_ODR_HIGH        (GPIOC_SD_CLK          ) |\
                                     PIN_ODR_HIGH        (GPIOC_PIN13                ) |\
                                     PIN_ODR_HIGH        (GPIOC_PIN14                ) |\
                                     PIN_ODR_HIGH        (GPIOC_PIN15                ))

#define VAL_GPIOC_AFRL              (PIN_AFIO_AF         ( GPIOC_PIN0  , 0) |\
                                     PIN_AFIO_AF         ( GPIOC_ETH_RMII_MDC  , 11) |\
                                     PIN_AFIO_AF         ( GPIOC_PIN2          , 0) |\
                                     PIN_AFIO_AF         ( GPIOC_PIN3           , 0) |\
                                     PIN_AFIO_AF         ( GPIOC_ETH_RMII_RXD0          , 11) |\
                                     PIN_AFIO_AF         ( GPIOC_ETH_RMII_RXD1          , 11) |\
                                     PIN_AFIO_AF         ( GPIOC_USART6_TX                 , 8) |\
                                     PIN_AFIO_AF         ( GPIOC_USART6_RX                 , 8))

#define VAL_GPIOC_AFRH              (PIN_AFIO_AF         ( GPIOC_SD_D0                 , 12)|\
                                     PIN_AFIO_AF         ( GPIOC_SD_D1          , 12) |\
                                     PIN_AFIO_AF         ( GPIOC_SD_D2           , 12) |\
                                     PIN_AFIO_AF         ( GPIOC_SD_D3                , 12) |\
                                     PIN_AFIO_AF         ( GPIOC_SD_CLK          , 12) |\
                                     PIN_AFIO_AF         ( GPIOC_PIN13                , 0) |\
                                     PIN_AFIO_AF         ( GPIOC_PIN14                , 0) |\
                                     PIN_AFIO_AF         ( GPIOC_PIN15                , 0))

/*
 * GPIOD setup:
 *
 */
#define VAL_GPIOD_MODER             (PIN_MODE_OUTPUT     ( GPIOD_SD_VDD      )|\
                                     PIN_MODE_INPUT     ( GPIOD_PIN1             )|\
                                     PIN_MODE_ALTERNATE    ( GPIOD_SD_CMD )|\
                                     PIN_MODE_OUTPUT     ( GPIOD_EPHY_NRST             )|\
                                     PIN_MODE_INPUT    ( GPIOD_PIN4      )|\
                                     PIN_MODE_INPUT     ( GPIOD_PIN5             )|\
                                     PIN_MODE_INPUT     ( GPIOD_PIN6             )|\
                                     PIN_MODE_INPUT    ( GPIOD_PIN7        )|\
                                     PIN_MODE_OUTPUT    ( GPIOD_LED2        )|\
                                     PIN_MODE_OUTPUT     ( GPIOD_LED4             )|\
                                     PIN_MODE_OUTPUT     ( GPIOD_LED5            )|\
                                     PIN_MODE_OUTPUT    ( GPIOD_RGB_R         )|\
                                     PIN_MODE_INPUT    ( GPIOD_PIN12         )|\
                                     PIN_MODE_OUTPUT    ( GPIOD_RGB_B         )|\
                                     PIN_MODE_OUTPUT    ( GPIOD_RGB_G        )|\
                                     PIN_MODE_INPUT     ( GPIOD_PIN15            ))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL ( GPIOD_SD_VDD      )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN1             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_SD_CMD )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_EPHY_NRST             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN4      )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN5             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN6             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN7        )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_LED2        )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_LED4             )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_LED5            )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_RGB_R         )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN12         )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_RGB_B         )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_RGB_G        )|\
                                     PIN_OTYPE_PUSHPULL ( GPIOD_PIN15            ))

#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_100M    ( GPIOD_SD_VDD      )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN1             )|\
                                     PIN_OSPEED_100M    ( GPIOD_SD_CMD )|\
                                     PIN_OSPEED_100M    ( GPIOD_EPHY_NRST             )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN4      )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN5             )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN6             )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN7        )|\
                                     PIN_OSPEED_100M    ( GPIOD_LED2        )|\
                                     PIN_OSPEED_100M    ( GPIOD_LED4             )|\
                                     PIN_OSPEED_100M    ( GPIOD_LED5            )|\
                                     PIN_OSPEED_100M    ( GPIOD_RGB_R         )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN12         )|\
                                     PIN_OSPEED_100M    ( GPIOD_RGB_B         )|\
                                     PIN_OSPEED_100M    ( GPIOD_RGB_G        )|\
                                     PIN_OSPEED_100M    ( GPIOD_PIN15            ))

#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP ( GPIOD_SD_VDD      )|\
                                     PIN_PUPDR_PULLDOWN ( GPIOD_PIN1             )|\
                                     PIN_PUPDR_FLOATING ( GPIOD_SD_CMD )|\
                                     PIN_PUPDR_PULLDOWN ( GPIOD_EPHY_NRST             )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_PIN4      )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_PIN5             )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_PIN6             )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_PIN7        )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_LED2        )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_LED4             )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_LED5            )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_RGB_R         )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_PIN12         )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_RGB_B         )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_RGB_G        )|\
                                     PIN_PUPDR_PULLUP ( GPIOD_PIN15            ))

#define VAL_GPIOD_ODR               (PIN_ODR_HIGH       ( GPIOD_SD_VDD      )|\
                                     PIN_ODR_HIGH       ( GPIOD_PIN1             )|\
                                     PIN_ODR_HIGH       ( GPIOD_SD_CMD )|\
                                     PIN_ODR_HIGH       ( GPIOD_EPHY_NRST             )|\
                                     PIN_ODR_HIGH        ( GPIOD_PIN4      )|\
                                     PIN_ODR_HIGH       ( GPIOD_PIN5             )|\
                                     PIN_ODR_HIGH       ( GPIOD_PIN6             )|\
                                     PIN_ODR_HIGH       ( GPIOD_PIN7        )|\
                                     PIN_ODR_HIGH       ( GPIOD_LED2        )|\
                                     PIN_ODR_HIGH       ( GPIOD_LED4             )|\
                                     PIN_ODR_HIGH       ( GPIOD_LED5            )|\
                                     PIN_ODR_HIGH       ( GPIOD_RGB_R         )|\
                                     PIN_ODR_HIGH       ( GPIOD_PIN12         )|\
                                     PIN_ODR_HIGH       ( GPIOD_RGB_B         )|\
                                     PIN_ODR_HIGH       ( GPIOD_RGB_G        )|\
                                     PIN_ODR_HIGH       ( GPIOD_PIN15            ))

#define VAL_GPIOD_AFRL              (PIN_AFIO_AF        ( GPIOD_SD_VDD      , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN1             , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_SD_CMD , 12) |\
                                     PIN_AFIO_AF        ( GPIOD_EPHY_NRST             , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN4      , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN5             , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN6             , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN7        , 0))

#define VAL_GPIOD_AFRH              (PIN_AFIO_AF        ( GPIOD_LED2        , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_LED4             , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_LED5            , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_RGB_R         , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN12         , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_RGB_B         , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_RGB_G        , 0) |\
                                     PIN_AFIO_AF        ( GPIOD_PIN15            , 0))

/*
 * GPIOE setup:
 *
 */
#define VAL_GPIOE_MODER             (PIN_MODE_INPUT(GPIOE_PIN0   ) |\
                                     PIN_MODE_INPUT(GPIOE_PIN1   ) |\
                                     PIN_MODE_INPUT(GPIOE_PIN2   ) |\
                                     PIN_MODE_INPUT(GPIOE_PIN3   ) |\
                                     PIN_MODE_INPUT(GPIOE_PIN4           ) |\
                                     PIN_MODE_INPUT(GPIOE_PIN5   ) |\
                                     PIN_MODE_INPUT(GPIOE_PIN6   ) |\
                                     PIN_MODE_INPUT (GPIOE_TIMESYNC   ) |\
                                     PIN_MODE_INPUT (GPIOE_PIN8  ) |\
                                     PIN_MODE_INPUT (GPIOE_DATASYNC  ) |\
                                     PIN_MODE_OUTPUT (GPIOE_MAX_SHDN ) |\
                                     PIN_MODE_OUTPUT (GPIOE_MAX_IDLE ) |\
                                     PIN_MODE_OUTPUT (GPIOE_MAX_CFG_CS          ) |\
                                     PIN_MODE_INPUT (GPIOE_MAX_CFG_CLK ) |\
                                     PIN_MODE_INPUT (GPIOE_MAX_CFG_MOSI ) |\
                                     PIN_MODE_INPUT (GPIOE_PIN15 ))

#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_PIN0   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN1   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN2   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4           ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_TIMESYNC   ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN8  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_DATASYNC  ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_MAX_SHDN ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_MAX_IDLE ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_MAX_CFG_CS          ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_MAX_CFG_CLK ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_MAX_CFG_MOSI ) |\
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN15 ))

#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_100M   (GPIOE_PIN0   ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN1   ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN2   ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN3   ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN4           ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN5   ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN6   ) |\
                                     PIN_OSPEED_100M   (GPIOE_TIMESYNC   ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN8  ) |\
                                     PIN_OSPEED_100M   (GPIOE_DATASYNC  ) |\
                                     PIN_OSPEED_100M   (GPIOE_MAX_SHDN ) |\
                                     PIN_OSPEED_100M   (GPIOE_MAX_IDLE ) |\
                                     PIN_OSPEED_100M   (GPIOE_MAX_CFG_CS          ) |\
                                     PIN_OSPEED_100M   (GPIOE_MAX_CFG_CLK ) |\
                                     PIN_OSPEED_100M   (GPIOE_MAX_CFG_MOSI ) |\
                                     PIN_OSPEED_100M   (GPIOE_PIN15 ))

#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP  (GPIOE_PIN0   ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN1   ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN2   ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN3   ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN4           ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN5   ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN6   ) |\
                                     PIN_PUPDR_PULLDOWN(GPIOE_TIMESYNC   ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN8  ) |\
                                     PIN_PUPDR_PULLDOWN(GPIOE_DATASYNC  ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_MAX_SHDN ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_MAX_IDLE ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_MAX_CFG_CS          ) |\
                                     PIN_PUPDR_FLOATING  (GPIOE_MAX_CFG_CLK ) |\
                                     PIN_PUPDR_FLOATING  (GPIOE_MAX_CFG_MOSI ) |\
                                     PIN_PUPDR_PULLUP  (GPIOE_PIN15 ))

#define VAL_GPIOE_ODR               (PIN_ODR_HIGH      (GPIOE_PIN0   ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN1   ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN2   ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN3   ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN4           ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN5   ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN6   ) |\
                                     PIN_ODR_HIGH      (GPIOE_TIMESYNC   ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN8  ) |\
                                     PIN_ODR_HIGH      (GPIOE_DATASYNC  ) |\
                                     PIN_ODR_HIGH      (GPIOE_MAX_SHDN ) |\
                                     PIN_ODR_HIGH      (GPIOE_MAX_IDLE ) |\
                                     PIN_ODR_HIGH      (GPIOE_MAX_CFG_CS          ) |\
                                     PIN_ODR_HIGH      (GPIOE_MAX_CFG_CLK ) |\
                                     PIN_ODR_HIGH      (GPIOE_MAX_CFG_MOSI ) |\
                                     PIN_ODR_HIGH      (GPIOE_PIN15 ))

#define VAL_GPIOE_AFRL              (PIN_AFIO_AF       (GPIOE_PIN0  , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN1  , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN2  , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN3  , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN4          , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN5  , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN6  , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_TIMESYNC  , 0 ))

#define VAL_GPIOE_AFRH              (PIN_AFIO_AF       (GPIOE_PIN8 , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_DATASYNC , 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_MAX_SHDN, 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_MAX_IDLE, 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_MAX_CFG_CS         , 5 ) |\
                                     PIN_AFIO_AF       (GPIOE_MAX_CFG_CLK, 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_MAX_CFG_MOSI, 0 ) |\
                                     PIN_AFIO_AF       (GPIOE_PIN15, 0 ))

#define VAL_GPIOF_MODER    0
#define VAL_GPIOF_OTYPER   0
#define VAL_GPIOF_OSPEEDR  0
#define VAL_GPIOF_PUPDR    0
#define VAL_GPIOF_ODR     0
#define VAL_GPIOF_AFRL 0
#define VAL_GPIOF_AFRH 0
#define VAL_GPIOG_MODER 0
#define VAL_GPIOG_OTYPER 0
#define VAL_GPIOG_OSPEEDR 0
#define VAL_GPIOG_PUPDR 0
#define VAL_GPIOG_ODR 0
#define VAL_GPIOG_AFRL 0
#define VAL_GPIOG_AFRH 0

#define VAL_GPIOH_MODER             (PIN_MODE_INPUT(GPIOH_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOH_OSC_OUT))

#define VAL_GPIOH_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOH_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOH_OSC_OUT))

#define VAL_GPIOH_OSPEEDR           (PIN_OSPEED_100M(GPIOH_OSC_IN) |        \
                                     PIN_OSPEED_100M(GPIOH_OSC_OUT))

#define VAL_GPIOH_PUPDR             (PIN_PUPDR_FLOATING(GPIOH_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_OSC_OUT))

#define VAL_GPIOH_ODR               (PIN_ODR_HIGH(GPIOH_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIOH_OSC_OUT))

#define VAL_GPIOH_AFRL              (PIN_AFIO_AF(GPIOH_OSC_IN, 0) |         \
                                     PIN_AFIO_AF(GPIOH_OSC_OUT, 0))

#define VAL_GPIOH_AFRH               0

#define VAL_GPIOI_MODER   0
#define VAL_GPIOI_OTYPER  0
#define VAL_GPIOI_OSPEEDR 0
#define VAL_GPIOI_PUPDR   0
#define VAL_GPIOI_ODR     0
#define VAL_GPIOI_AFRL    0
#define VAL_GPIOI_AFRH    0

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
