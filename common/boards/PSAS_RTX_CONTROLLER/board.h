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
 * Setup for Olimex STM32-E407 board.
 */

/*
 * Board identifier.
 */
#define BOARD_PSAS_RTX_CONTROLLER
#define BOARD_NAME                  "RTx Controller"

/*
 * Ethernet PHY type.
 */
#define BOARD_PHY_ID                MII_KS8721_ID
#define BOARD_PHY_RMII

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                12000000
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
#define GPIOA_TP7	           		0
#define GPIOA_ETH_RMII_REF_CLK      1
#define GPIOA_ETH_RMII_MDIO         2
#define GPIOA_ETH_RMII_MDINT        3
#define GPIOA_ADC_CNV               4
#define GPIOA_SPI1_SCK              5
#define GPIOA_SPI1_MISO             6
#define GPIOA_ETH_RMII_CRS_DV       7
#define GPIOA_ETH_RMII_MCO1         8
#define GPIOA_OTG_FS_VBUS           9
#define GPIOA_OTG_FS_ID             10
#define GPIOA_OTG_FS_DM             11
#define GPIOA_OTG_FS_DP             12
#define GPIOA_JTAG_TMS              13
#define GPIOA_JTAG_TCK              14
#define GPIOA_JTAG_TDI              15

#define GPIOB_PIN0      			0
#define GPIOB_PIN1		          	1
#define GPIOB_BOOT1                 2
#define GPIOB_JTAG_TDO              3
#define GPIOB_JTAG_TRST             4
#define GPIOB_PIN5                  5
#define GPIOB_PIN6                  6
#define GPIOB_PIN7                  7
#define GPIOB_PIN8              	8
#define GPIOB_PIN9		            9
#define GPIOB_PIN10                 10
#define GPIOB_ETH_RMII_TXEN         11
#define GPIOB_ETH_RMII_TXD0         12
#define GPIOB_ETH_RMII_TXD1         13
#define GPIOB_TP6                   14
#define GPIOB_PIN15                 15

#define GPIOC_VREF_MONITOR          0
#define GPIOC_ETH_RMII_MDC          1
#define GPIOC_PIN2                  2
#define GPIOC_PIN3                  3
#define GPIOC_ETH_RMII_RXD0         4
#define GPIOC_ETH_RMII_RXD1         5
#define GPIOC_PWM_AXIS3_V           6
#define GPIOC_PWM_AXIS3_U           7
#define GPIOC_PWM_AXIS4_V           8
#define GPIOC_PWM_AXIS4_U           9
#define GPIOC_PIN10                 10
#define GPIOC_PIN11                 11
#define GPIOC_TP3                   12
#define GPIOC_PIN13                 13
#define GPIOC_TP4                   14
#define GPIOC_PIN15                 15

#define GPIOD_AXIS3_EN              0
#define GPIOD_AXIS3_WD              1
#define GPIOD_AXIS4_EN              2
#define GPIOD_AXIS4_WD              3
#define GPIOD_PIN4                  4
#define GPIOD_VERT_WD               5
#define GPIOD_VERT_EN               6
#define GPIOD_PIN7                  7
#define GPIOD_PIN8                  8
#define GPIOD_PIN9                  9
#define GPIOD_LAT_WD                10
#define GPIOD_LAT_EN                11
#define GPIOD_VERT_V                12
#define GPIOD_VERT_U                13
#define GPIOD_LAT_V                 14
#define GPIOD_LAT_U                 15

#define GPIOE_PIN0                  0
#define GPIOE_PIN1                  1
#define GPIOE_PIN2                  2
#define GPIOE_PIN3                  3
#define GPIOE_PIN4                  4
#define GPIOE_PIN5                  5
#define GPIOE_PIN6                  6
#define GPIOE_CONV_DONE             7
#define GPIOE_PIN8                  8
#define GPIOE_PIN9                  9
#define GPIOE_PIN10                 10
#define GPIOE_PIN11                 11
#define GPIOE_PIN12                 12
#define GPIOE_PIN13                 13
#define GPIOE_PIN14                 14
#define GPIOE_PIN15                 15

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
 * GPIOA setup:
 *
 * PA0  - BUTTON_WKUP               (input floating).
 * PA1  - ETH_RMII_REF_CLK          (alternate 11).
 * PA2  - ETH_RMII_MDIO             (alternate 11).
 * PA3  - ETH_RMII_MDINT            (input floating).
 * PA4  - PIN4                      (input pullup).
 * PA5  - PIN5                      (input pullup).
 * PA6  - PIN6                      (input pullup).
 * PA7  - ETH_RMII_CRS_DV           (alternate 11).
 * PA8  - USB_HS_BUSON              (output pushpull maximum).
 * PA9  - OTG_FS_VBUS               (input pulldown).
 * PA10 - OTG_FS_ID                 (alternate 10).
 * PA11 - OTG_FS_DM                 (alternate 10).
 * PA12 - OTG_FS_DP                 (alternate 10).
 * PA13 - JTAG_TMS                  (alternate 0).
 * PA14 - JTAG_TCK                  (alternate 0).
 * PA15 - JTAG_TDI                  (alternate 0).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_INPUT(GPIOA_TP7) |    \
                                     PIN_MODE_ALTERNATE(GPIOA_ETH_RMII_REF_CLK) |\
                                     PIN_MODE_ALTERNATE(GPIOA_ETH_RMII_MDIO) |\
                                     PIN_MODE_INPUT(GPIOA_ETH_RMII_MDINT) | \
                                     PIN_MODE_INPUT(GPIOA_ADC_CNV) |           \
                                     PIN_MODE_INPUT(GPIOA_SPI1_SCK) |           \
                                     PIN_MODE_INPUT(GPIOA_SPI1_MISO) |           \
                                     PIN_MODE_ALTERNATE(GPIOA_ETH_RMII_CRS_DV) |\
                                     PIN_MODE_OUTPUT(GPIOA_ETH_RMII_MCO1) |  \
                                     PIN_MODE_INPUT(GPIOA_OTG_FS_VBUS) |    \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_ID) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DM) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DP) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_JTAG_TMS) |   \
                                     PIN_MODE_ALTERNATE(GPIOA_JTAG_TCK) |   \
                                     PIN_MODE_ALTERNATE(GPIOA_JTAG_TDI))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_TP7) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_REF_CLK) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_MDIO) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_MDINT) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ADC_CNV) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SPI1_SCK) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SPI1_MISO) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_CRS_DV) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_ETH_RMII_MCO1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_VBUS) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_ID) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DM) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DP) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTAG_TMS) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTAG_TCK) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTAG_TDI))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_100M(GPIOA_TP7) |   \
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_REF_CLK) |\
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_MDIO) | \
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_MDINT) |\
                                     PIN_OSPEED_100M(GPIOA_ADC_CNV) |          \
                                     PIN_OSPEED_100M(GPIOA_SPI1_SCK) |          \
                                     PIN_OSPEED_100M(GPIOA_SPI1_MISO) |          \
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_CRS_DV) |\
                                     PIN_OSPEED_100M(GPIOA_ETH_RMII_MCO1) |  \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_VBUS) |   \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_ID) |     \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DM) |     \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DP) |     \
                                     PIN_OSPEED_100M(GPIOA_JTAG_TMS) |      \
                                     PIN_OSPEED_100M(GPIOA_JTAG_TCK) |      \
                                     PIN_OSPEED_100M(GPIOA_JTAG_TDI))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_TP7) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_REF_CLK) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_MDIO) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_MDINT) |\
                                     PIN_PUPDR_PULLUP(GPIOA_ADC_CNV) |         \
                                     PIN_PUPDR_PULLUP(GPIOA_SPI1_SCK) |         \
                                     PIN_PUPDR_PULLUP(GPIOA_SPI1_MISO) |         \
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_CRS_DV) |\
                                     PIN_PUPDR_FLOATING(GPIOA_ETH_RMII_MCO1) |\
                                     PIN_PUPDR_PULLDOWN(GPIOA_OTG_FS_VBUS) |\
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_ID) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DM) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DP) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_JTAG_TMS) |   \
                                     PIN_PUPDR_PULLDOWN(GPIOA_JTAG_TCK) |   \
                                     PIN_PUPDR_FLOATING(GPIOA_JTAG_TDI))
#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_TP7) |      \
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_REF_CLK) | \
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_MDIO) |    \
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_MDINT) |   \
                                     PIN_ODR_HIGH(GPIOA_ADC_CNV) |             \
                                     PIN_ODR_HIGH(GPIOA_SPI1_SCK) |             \
                                     PIN_ODR_HIGH(GPIOA_SPI1_MISO) |             \
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_CRS_DV) |  \
                                     PIN_ODR_HIGH(GPIOA_ETH_RMII_MCO1) |     \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_VBUS) |      \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_ID) |        \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DM) |        \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DP) |        \
                                     PIN_ODR_HIGH(GPIOA_JTAG_TMS) |         \
                                     PIN_ODR_HIGH(GPIOA_JTAG_TCK) |         \
                                     PIN_ODR_HIGH(GPIOA_JTAG_TDI))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_TP7, 0) |    \
                                     PIN_AFIO_AF(GPIOA_ETH_RMII_REF_CLK, 11) |\
                                     PIN_AFIO_AF(GPIOA_ETH_RMII_MDIO, 11) | \
                                     PIN_AFIO_AF(GPIOA_ETH_RMII_MDINT, 0) | \
                                     PIN_AFIO_AF(GPIOA_ADC_CNV, 0) |           \
                                     PIN_AFIO_AF(GPIOA_SPI1_SCK, 0) |           \
                                     PIN_AFIO_AF(GPIOA_SPI1_MISO, 0) |           \
                                     PIN_AFIO_AF(GPIOA_ETH_RMII_CRS_DV, 11))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_ETH_RMII_MCO1, 0) |   \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_VBUS, 0) |    \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_ID, 10) |     \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_DM, 10) |     \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_DP, 10) |     \
                                     PIN_AFIO_AF(GPIOA_JTAG_TMS, 0) |       \
                                     PIN_AFIO_AF(GPIOA_JTAG_TCK, 0) |       \
                                     PIN_AFIO_AF(GPIOA_JTAG_TDI, 0))

/*
 * GPIOB setup:
 *
 * PB0  - USB_FS_BUSON              (output pushpull maximum).
 * PB1  - USB_HS_FAULT              (input floating).
 * PB2  - BOOT1                     (input floating).
 * PB3  - JTAG_TDO                  (alternate 0).
 * PB4  - JTAG_TRST                 (alternate 0).
 * PB5  - PIN5                      (input pullup).
 * PB6  - PIN6                      (input pullup).
 * PB7  - PIN7                      (input pullup).
 * PB8  - I2C1_SCL                  (alternate 4).
 * PB9  - I2C1_SDA                  (alternate 4).
 * PB10 - SPI2_SCK                  (alternate 5).
 * PB11 - PIN11                     (input pullup).
 * PB12 - OTG_HS_ID                 (alternate 12).
 * PB13 - OTG_HS_VBUS               (input pulldown).
 * PB14 - OTG_HS_DM                 (alternate 12).
 * PB15 - OTG_HS_DP                 (alternate 12).
 */
#define VAL_GPIOB_MODER             (PIN_MODE_OUTPUT(GPIOB_PIN0) |  \
                                     PIN_MODE_INPUT(GPIOB_PIN1) |   \
                                     PIN_MODE_INPUT(GPIOB_BOOT1) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_JTAG_TDO) |   \
                                     PIN_MODE_ALTERNATE(GPIOB_JTAG_TRST) |  \
                                     PIN_MODE_INPUT(GPIOB_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN6) |           \
                                     PIN_MODE_OUTPUT(GPIOB_PIN7) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_PIN8) |   \
                                     PIN_MODE_ALTERNATE(GPIOB_PIN9) |   \
                                     PIN_MODE_ALTERNATE(GPIOB_PIN10) |   \
                                     PIN_MODE_INPUT(GPIOB_ETH_RMII_TXEN) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_ETH_RMII_TXD0) |  \
                                     PIN_MODE_INPUT(GPIOB_ETH_RMII_TXD1) |    \
                                     PIN_MODE_ALTERNATE(GPIOB_TP6) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_PIN15))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_PIN0) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_BOOT1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_JTAG_TDO) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_JTAG_TRST) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN7) |       \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_PIN8) |  \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_PIN9) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN10) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ETH_RMII_TXEN) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ETH_RMII_TXD0) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ETH_RMII_TXD1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_TP6) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN15))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_100M(GPIOB_PIN0) |  \
                                     PIN_OSPEED_100M(GPIOB_PIN1) |  \
                                     PIN_OSPEED_100M(GPIOB_BOOT1) |         \
                                     PIN_OSPEED_100M(GPIOB_JTAG_TDO) |      \
                                     PIN_OSPEED_100M(GPIOB_JTAG_TRST) |     \
                                     PIN_OSPEED_100M(GPIOB_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN8) |      \
                                     PIN_OSPEED_100M(GPIOB_PIN9) |      \
                                     PIN_OSPEED_100M(GPIOB_PIN10) |      \
                                     PIN_OSPEED_100M(GPIOB_ETH_RMII_TXEN) |         \
                                     PIN_OSPEED_100M(GPIOB_ETH_RMII_TXD0) |     \
                                     PIN_OSPEED_100M(GPIOB_ETH_RMII_TXD1) |   \
                                     PIN_OSPEED_100M(GPIOB_TP6) |     \
                                     PIN_OSPEED_100M(GPIOB_PIN15))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_PIN0) |\
                                     PIN_PUPDR_FLOATING(GPIOB_PIN1) |\
                                     PIN_PUPDR_FLOATING(GPIOB_BOOT1) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_JTAG_TDO) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_JTAG_TRST) |  \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN7) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN8) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN9) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN10) |   \
                                     PIN_PUPDR_PULLUP(GPIOB_ETH_RMII_TXEN) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_ETH_RMII_TXD0) |  \
                                     PIN_PUPDR_PULLDOWN(GPIOB_ETH_RMII_TXD1) |\
                                     PIN_PUPDR_FLOATING(GPIOB_TP6) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN15))
#define VAL_GPIOB_ODR               (PIN_ODR_HIGH(GPIOB_PIN0) |     \
                                     PIN_ODR_HIGH(GPIOB_PIN1) |     \
                                     PIN_ODR_HIGH(GPIOB_BOOT1) |            \
                                     PIN_ODR_HIGH(GPIOB_JTAG_TDO) |         \
                                     PIN_ODR_HIGH(GPIOB_JTAG_TRST) |        \
                                     PIN_ODR_HIGH(GPIOB_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN8) |         \
                                     PIN_ODR_HIGH(GPIOB_PIN9) |         \
                                     PIN_ODR_HIGH(GPIOB_PIN10) |         \
                                     PIN_ODR_HIGH(GPIOB_ETH_RMII_TXEN) |            \
                                     PIN_ODR_HIGH(GPIOB_ETH_RMII_TXD0) |        \
                                     PIN_ODR_HIGH(GPIOB_ETH_RMII_TXD1) |      \
                                     PIN_ODR_HIGH(GPIOB_TP6) |        \
                                     PIN_ODR_HIGH(GPIOB_PIN15))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_PIN0, 0) |   \
                                     PIN_AFIO_AF(GPIOB_PIN1, 0) |   \
                                     PIN_AFIO_AF(GPIOB_BOOT1, 0) |          \
                                     PIN_AFIO_AF(GPIOB_JTAG_TDO, 0) |       \
                                     PIN_AFIO_AF(GPIOB_JTAG_TRST, 0) |      \
                                     PIN_AFIO_AF(GPIOB_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN7, 0))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_PIN8, 4) |       \
                                     PIN_AFIO_AF(GPIOB_PIN9, 4) |       \
                                     PIN_AFIO_AF(GPIOB_PIN10, 5) |       \
                                     PIN_AFIO_AF(GPIOB_ETH_RMII_TXEN, 0) |          \
                                     PIN_AFIO_AF(GPIOB_ETH_RMII_TXD0, 12) |     \
                                     PIN_AFIO_AF(GPIOB_ETH_RMII_TXD1, 0) |    \
                                     PIN_AFIO_AF(GPIOB_TP6, 12) |     \
                                     PIN_AFIO_AF(GPIOB_PIN15, 12))

/*
 * GPIOC setup:
 *
 * PC0  - PIN0                      (input pullup).
 * PC1  - ETH_RMII_MDC              (alternate 11).
 * PC2  - SPI2_MISO                 (alternate 5).
 * PC3  - SPI2_MOSI                 (alternate 5).
 * PC4  - ETH_RMII_RXD0             (alternate 11).
 * PC5  - ETH_RMII_RXD1             (alternate 11).
 * PC6  - USART6_TX                 (alternate 8).
 * PC7  - USART6_RX                 (alternate 8).
 * PC8  - SD_D0                     (alternate 12).
 * PC9  - SD_D1                     (alternate 12).
 * PC10 - SD_D2                     (alternate 12).
 * PC11 - SD_D3                     (alternate 12).
 * PC12 - SD_CLK                    (alternate 12).
 * PC13 - LED                       (output pushpull maximum).
 * PC14 - OSC32_IN                  (input floating).
 * PC15 - OSC32_OUT                 (input floating).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_VREF_MONITOR) |           \
                                     PIN_MODE_ALTERNATE(GPIOC_ETH_RMII_MDC) |\
                                     PIN_MODE_ALTERNATE(GPIOC_PIN2) |  \
                                     PIN_MODE_ALTERNATE(GPIOC_PIN3) |  \
                                     PIN_MODE_ALTERNATE(GPIOC_ETH_RMII_RXD0) |\
                                     PIN_MODE_ALTERNATE(GPIOC_ETH_RMII_RXD1) |\
                                     PIN_MODE_ALTERNATE(GPIOC_PWM_AXIS3_V) |  \
                                     PIN_MODE_ALTERNATE(GPIOC_PWM_AXIS3_U) |  \
                                     PIN_MODE_ALTERNATE(GPIOC_PWM_AXIS4_V) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_PWM_AXIS4_U) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_PIN10) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_PIN11) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_TP3) |     \
                                     PIN_MODE_OUTPUT(GPIOC_PIN13) |           \
                                     PIN_MODE_INPUT(GPIOC_TP4) |       \
                                     PIN_MODE_INPUT(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_VREF_MONITOR) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ETH_RMII_MDC) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ETH_RMII_RXD0) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_ETH_RMII_RXD1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PWM_AXIS3_V) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PWM_AXIS3_U) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PWM_AXIS4_V) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PWM_AXIS4_U) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_TP3) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOC_TP4) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_100M(GPIOC_VREF_MONITOR) |          \
                                     PIN_OSPEED_100M(GPIOC_ETH_RMII_MDC) |  \
                                     PIN_OSPEED_100M(GPIOC_PIN2) |     \
                                     PIN_OSPEED_100M(GPIOC_PIN3) |     \
                                     PIN_OSPEED_100M(GPIOC_ETH_RMII_RXD0) | \
                                     PIN_OSPEED_100M(GPIOC_ETH_RMII_RXD1) | \
                                     PIN_OSPEED_100M(GPIOC_PWM_AXIS3_V) |     \
                                     PIN_OSPEED_100M(GPIOC_PWM_AXIS3_U) |     \
                                     PIN_OSPEED_100M(GPIOC_PWM_AXIS4_V) |         \
                                     PIN_OSPEED_100M(GPIOC_PWM_AXIS4_U) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOC_TP3) |        \
                                     PIN_OSPEED_100M(GPIOC_PIN13) |           \
                                     PIN_OSPEED_100M(GPIOC_TP4) |      \
                                     PIN_OSPEED_100M(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_VREF_MONITOR) |         \
                                     PIN_PUPDR_FLOATING(GPIOC_ETH_RMII_MDC) |\
                                     PIN_PUPDR_FLOATING(GPIOC_PIN2) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN3) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_ETH_RMII_RXD0) |\
                                     PIN_PUPDR_FLOATING(GPIOC_ETH_RMII_RXD1) |\
                                     PIN_PUPDR_FLOATING(GPIOC_PWM_AXIS3_V) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_PWM_AXIS3_U) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_PWM_AXIS4_V) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PWM_AXIS4_U) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_TP3) |     \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN13) |        \
                                     PIN_PUPDR_FLOATING(GPIOC_TP4) |   \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_VREF_MONITOR) |             \
                                     PIN_ODR_HIGH(GPIOC_ETH_RMII_MDC) |     \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |        \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |        \
                                     PIN_ODR_HIGH(GPIOC_ETH_RMII_RXD0) |    \
                                     PIN_ODR_HIGH(GPIOC_ETH_RMII_RXD1) |    \
                                     PIN_ODR_HIGH(GPIOC_PWM_AXIS3_V) |        \
                                     PIN_ODR_HIGH(GPIOC_PWM_AXIS3_U) |        \
                                     PIN_ODR_HIGH(GPIOC_PWM_AXIS4_V) |            \
                                     PIN_ODR_HIGH(GPIOC_PWM_AXIS4_U) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOC_TP3) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN13) |              \
                                     PIN_ODR_HIGH(GPIOC_TP4) |         \
                                     PIN_ODR_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_VREF_MONITOR, 0) |           \
                                     PIN_AFIO_AF(GPIOC_ETH_RMII_MDC, 11) |  \
                                     PIN_AFIO_AF(GPIOC_PIN2, 5) |      \
                                     PIN_AFIO_AF(GPIOC_PIN3, 5) |      \
                                     PIN_AFIO_AF(GPIOC_ETH_RMII_RXD0, 11) | \
                                     PIN_AFIO_AF(GPIOC_ETH_RMII_RXD1, 11) | \
                                     PIN_AFIO_AF(GPIOC_PWM_AXIS3_V, 8) |      \
                                     PIN_AFIO_AF(GPIOC_PWM_AXIS3_U, 8))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PWM_AXIS4_V, 12) |         \
                                     PIN_AFIO_AF(GPIOC_PWM_AXIS4_U, 12) |         \
                                     PIN_AFIO_AF(GPIOC_PIN10, 12) |         \
                                     PIN_AFIO_AF(GPIOC_PIN11, 12) |         \
                                     PIN_AFIO_AF(GPIOC_TP3, 12) |        \
                                     PIN_AFIO_AF(GPIOC_PIN13, 0) |            \
                                     PIN_AFIO_AF(GPIOC_TP4, 0) |       \
                                     PIN_AFIO_AF(GPIOC_PIN15, 0))

/*
 * GPIOD setup:
 *
 * PD0  - PIN0                      (input pullup).
 * PD1  - PIN1                      (input pullup).
 * PD2  - SD_CMD                    (alternate 12).
 * PD3  - PIN3                      (input pullup).
 * PD4  - PIN4                      (input pullup).
 * PD5  - PIN5                      (input pullup).
 * PD6  - PIN6                      (input pullup).
 * PD7  - PIN7                      (input pullup).
 * PD8  - PIN8                      (input pullup).
 * PD9  - PIN9                      (input pullup).
 * PD10 - PIN10                     (input pullup).
 * PD11 - PIN11                     (input pullup).
 * PD12 - PIN12                     (input pullup).
 * PD13 - PIN13                     (input pullup).
 * PD14 - PIN14                     (input pullup).
 * PD15 - PIN15                     (input pullup).
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_AXIS3_EN) |           \
                                     PIN_MODE_INPUT(GPIOD_AXIS3_WD) |           \
                                     PIN_MODE_ALTERNATE(GPIOD_AXIS4_EN) |     \
                                     PIN_MODE_INPUT(GPIOD_AXIS4_WD) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOD_VERT_WD) |           \
                                     PIN_MODE_INPUT(GPIOD_VERT_EN) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOD_LAT_WD) |          \
                                     PIN_MODE_INPUT(GPIOD_LAT_EN) |          \
                                     PIN_MODE_INPUT(GPIOD_VERT_V) |          \
                                     PIN_MODE_INPUT(GPIOD_VERT_U) |          \
                                     PIN_MODE_INPUT(GPIOD_LAT_V) |          \
                                     PIN_MODE_OUTPUT(GPIOD_LAT_U))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_AXIS3_EN) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_AXIS3_WD) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_AXIS4_EN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_AXIS4_WD) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_VERT_WD) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_VERT_EN) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_LAT_WD) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_LAT_EN) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_VERT_V) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_VERT_U) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_LAT_V) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_LAT_U))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_100M(GPIOD_AXIS3_EN) |          \
                                     PIN_OSPEED_100M(GPIOD_AXIS3_WD) |          \
                                     PIN_OSPEED_100M(GPIOD_AXIS4_EN) |        \
                                     PIN_OSPEED_100M(GPIOD_AXIS4_WD) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOD_VERT_WD) |          \
                                     PIN_OSPEED_100M(GPIOD_VERT_EN) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOD_LAT_WD) |         \
                                     PIN_OSPEED_100M(GPIOD_LAT_EN) |         \
                                     PIN_OSPEED_100M(GPIOD_VERT_V) |         \
                                     PIN_OSPEED_100M(GPIOD_VERT_U) |         \
                                     PIN_OSPEED_100M(GPIOD_LAT_V) |         \
                                     PIN_OSPEED_100M(GPIOD_LAT_U))
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP(GPIOD_AXIS3_EN) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_AXIS3_WD) |         \
                                     PIN_PUPDR_FLOATING(GPIOD_AXIS4_EN) |     \
                                     PIN_PUPDR_PULLUP(GPIOD_AXIS4_WD) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_VERT_WD) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_VERT_EN) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_LAT_WD) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_LAT_EN) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_VERT_V) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_VERT_U) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_LAT_V) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_LAT_U))
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_AXIS3_EN) |             \
                                     PIN_ODR_HIGH(GPIOD_AXIS3_WD) |             \
                                     PIN_ODR_HIGH(GPIOD_AXIS4_EN) |           \
                                     PIN_ODR_HIGH(GPIOD_AXIS4_WD) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOD_VERT_WD) |             \
                                     PIN_ODR_HIGH(GPIOD_VERT_EN) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOD_LAT_WD) |            \
                                     PIN_ODR_HIGH(GPIOD_LAT_EN) |            \
                                     PIN_ODR_HIGH(GPIOD_VERT_V) |            \
                                     PIN_ODR_HIGH(GPIOD_VERT_U) |            \
                                     PIN_ODR_HIGH(GPIOD_LAT_V) |            \
                                     PIN_ODR_HIGH(GPIOD_LAT_U))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_AXIS3_EN, 0) |           \
                                     PIN_AFIO_AF(GPIOD_AXIS3_WD, 0) |           \
                                     PIN_AFIO_AF(GPIOD_AXIS4_EN, 12) |        \
                                     PIN_AFIO_AF(GPIOD_AXIS4_WD, 0) |           \
                                     PIN_AFIO_AF(GPIOD_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOD_VERT_WD, 0) |           \
                                     PIN_AFIO_AF(GPIOD_VERT_EN, 0) |           \
                                     PIN_AFIO_AF(GPIOD_PIN7, 0))
#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOD_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOD_LAT_WD, 0) |          \
                                     PIN_AFIO_AF(GPIOD_LAT_EN, 0) |          \
                                     PIN_AFIO_AF(GPIOD_VERT_V, 0) |          \
                                     PIN_AFIO_AF(GPIOD_VERT_U, 0) |          \
                                     PIN_AFIO_AF(GPIOD_LAT_V, 0) |          \
                                     PIN_AFIO_AF(GPIOD_LAT_U, 0))

/*
 * GPIOE setup:
 *
 * PE0  - PIN0                      (input pullup).
 * PE1  - PIN1                      (input pullup).
 * PE2  - PIN2                      (input pullup).
 * PE3  - PIN3                      (input pullup).
 * PE4  - PIN4                      (input pullup).
 * PE5  - PIN5                      (input pullup).
 * PE6  - PIN6                      (input pullup).
 * PE7  - PIN7                      (input pullup).
 * PE8  - PIN8                      (input pullup).
 * PE9  - PIN9                      (input pullup).
 * PE10 - PIN10                     (input pullup).
 * PE11 - PIN11                     (input pullup).
 * PE12 - PIN12                     (input pullup).
 * PE13 - PIN13                     (input pullup).
 * PE14 - PIN14                     (input pullup).
 * PE15 - PIN15                     (input pullup).
 */
#define VAL_GPIOE_MODER             (PIN_MODE_INPUT(GPIOE_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN1) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOE_CONV_DONE) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN15))
#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_CONV_DONE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN15))
#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_100M(GPIOE_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOE_CONV_DONE) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN15))
#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP(GPIOE_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_CONV_DONE) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN15))
#define VAL_GPIOE_ODR               (PIN_ODR_HIGH(GPIOE_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOE_CONV_DONE) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN15))
#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIOE_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOE_CONV_DONE, 0))
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN15, 0))

/*
 * GPIOH setup:
 *
 * PH0  - OSC_IN                    (input floating).
 * PH1  - OSC_OUT                   (input floating).
 * PH2  - PIN2                      (input pullup).
 * PH3  - PIN3                      (input pullup).
 * PH4  - PIN4                      (input pullup).
 * PH5  - PIN5                      (input pullup).
 * PH6  - PIN6                      (input pullup).
 * PH7  - PIN7                      (input pullup).
 * PH8  - PIN8                      (input pullup).
 * PH9  - PIN9                      (input pullup).
 * PH10 - PIN10                     (input pullup).
 * PH11 - PIN11                     (input pullup).
 * PH12 - PIN12                     (input pullup).
 * PH13 - PIN13                     (input pullup).
 * PH14 - PIN14                     (input pullup).
 * PH15 - PIN15                     (input pullup).
 */
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
#define VAL_GPIOH_AFRH

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
