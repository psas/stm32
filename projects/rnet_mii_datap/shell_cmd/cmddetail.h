/*! \file cmddetail.h
 *
 */


#ifndef _CMDDETAIL_H
#define _CMDDETAIL_H

/*!
 * \addtogroup cmddetail
 * @{
 */

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

#define           DEBUG_PHY                1
#define           DEBUG_KSZ                1

#if DEBUG_PHY

/* MII divider optimal value.*/
#if (STM32_HCLK >= 150000000)
#define MACMIIDR_CR ETH_MACMIIAR_CR_Div102
#elif (STM32_HCLK >= 100000000)
#define MACMIIDR_CR ETH_MACMIIAR_CR_Div62
#elif (STM32_HCLK >= 60000000)
#define MACMIIDR_CR     ETH_MACMIIAR_CR_Div42
#elif (STM32_HCLK >= 35000000)
#define MACMIIDR_CR     ETH_MACMIIAR_CR_Div26
#elif (STM32_HCLK >= 20000000)
#define MACMIIDR_CR     ETH_MACMIIAR_CR_Div16
#else
#error "STM32_HCLK below minimum frequency for ETH operations (20MHz)"
#endif

#define MII_BMCR                0x00    /**< Basic mode control register.   */


//
//uint32_t mii_read(MACDriver *macp, uint32_t reg) ;
//static void mii_write(MACDriver *macp, uint32_t reg, uint32_t value) ;
void cmd_phy(BaseSequentialStream *chp, int argc, char *argv[]) ;
#endif

#if DEBUG_KSZ
void cmd_ksz_nodes_en_n(BaseSequentialStream *chp, int argc, char *argv[]) ;
void cmd_ksz_n1en_n(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_ksz_rst_n(BaseSequentialStream *chp, int argc, char *argv[]) ;
void cmd_ksz_pwr(BaseSequentialStream *chp, int argc, char *argv[]) ;
void cmd_show(BaseSequentialStream *chp, int argc, char *argv[]) ;

#endif

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);



#ifdef __cplusplus
}
#endif


//! @}

#endif
