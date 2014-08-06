/*! \file main.c
 *
 *   Experiment with the MAX2769 configuration
 */

#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "utils_general.h"
#include "utils_led.h"
#include "MAX2769.h"
static 	int conf3_state       =  MAX2769_CONF3_DEF;

static void max2769_config(void) {
	int conf1       =  0;
	int conf2       =  0;
	int conf3       =  0;
	int pllconf     =  0;
	int pllidr      =  0;
	int cfdr        =  0;

	max2769_reset();
	// Configuration from Google doc MAX2769RegisterConfiguration
	// CONF1
	conf1 = (0b1            << MAX2769_CONF1_CHIPEN ) |
	        (0b0            << MAX2769_CONF1_IDLE   ) |
	        (0b1111         << MAX2769_CONF1_ILNA1  ) |
	        (0b00           << MAX2769_CONF1_ILNA2  ) |
	        (0b11           << MAX2769_CONF1_ILO    ) |
	        (0b11           << MAX2769_CONF1_IMIX   ) |
	        (0b0            << MAX2769_CONF1_MIXPOLE) |
	        (0b10           << MAX2769_CONF1_LNAMODE) |
	        (0b1            << MAX2769_CONF1_MIXEN  ) |
	        (0b0            << MAX2769_CONF1_ANTEN  ) |
	        (0b001101       << MAX2769_CONF1_FCEN   ) |
	        (0b00           << MAX2769_CONF1_FBW    ) |
	        (0b0            << MAX2769_CONF1_F3OR5  ) |
	        (0b1            << MAX2769_CONF1_FCENX  ) |
	        (0b1            << MAX2769_CONF1_FGAIN  );
	max2769_set(MAX2769_CONF1, conf1 );
	// CONF2
	conf2 = (0b1            << MAX2769_CONF2_IQEN    ) |
	        (0b10101010     << MAX2769_CONF2_GAINREF ) |
	        (0b00           << MAX2769_CONF2_AGCMODE ) |
	        (0b01           << MAX2769_CONF2_FORMAT  ) |
	        (0b010          << MAX2769_CONF2_BITS    ) |
	        (0b00           << MAX2769_CONF2_DRVCFG  ) |
	        (0b1            << MAX2769_CONF2_LOEN    ) |
	        (0b00           << MAX2769_CONF2_DIEID   );
	max2769_set(MAX2769_CONF2, conf2 );
	// CONF3
	conf3 = (0b111010       << MAX2769_CONF3_GAININ      ) |
	        (0b1            << MAX2769_CONF3_FSLOWEN     ) |
	        (0b0            << MAX2769_CONF3_HILOADEN    ) |
	        (0b1            << MAX2769_CONF3_ADCEN       ) |
	        (0b1            << MAX2769_CONF3_DRVEN       ) |
	        (0b1            << MAX2769_CONF3_FOFSTEN     ) |
	        (0b1            << MAX2769_CONF3_FILTEN      ) |
	        (0b1            << MAX2769_CONF3_FHIPEN      ) |
	        (0b1            << MAX2769_CONF3_PGAIEN      ) |
	        (0b1            << MAX2769_CONF3_PGAQEN      ) |
	        (0b1            << MAX2769_CONF3_STRMEN      ) |
	        (0b1            << MAX2769_CONF3_STRMSTART   ) |
	        (0b0            << MAX2769_CONF3_STRMSTOP    ) |
	        (0b000          << MAX2769_CONF3_STRMCOUNT   ) |
	        (0b11           << MAX2769_CONF3_STRMBITS    ) |
	        (0b0            << MAX2769_CONF3_STRMPEN     ) |
	        (0b1            << MAX2769_CONF3_TIMESYNCEN  ) |
	        (0b1            << MAX2769_CONF3_DATASYNCEN  ) |
	        (0b0            << MAX2769_CONF3_STRMRST     );
	max2769_set(MAX2769_CONF3, conf3 );
	conf3_state = conf3;
	// PLLCONF
	pllconf = (0b1          << MAX2769_PLL_VCOEN)    |
	          (0b0          << MAX2769_PLL_IVCO)     |
	          (0b1          << MAX2769_PLL_REFOUTEN) |
	          (0b01         << MAX2769_PLL_REFDIV)   |
	          (0b01         << MAX2769_PLL_IXTAL)    |
	          (0b1000       << MAX2769_PLL_XTALCAP)  |
	          (0b0000       << MAX2769_PLL_LDMUX)    |
	          (0b0          << MAX2769_PLL_ICP)      |
	          (0b0          << MAX2769_PLL_PFDEN)    |
	          (0b000        << MAX2769_PLL_CPTEST)   |
	          (0b1          << MAX2769_PLL_INT_PLL)  |
	          (0b0          << MAX2769_PLL_PWRSAV);
	max2769_set(MAX2769_PLLCONF, pllconf );
	// PLLIDR
	pllidr = (1536          << MAX2769_PLLIDR_NDIV) |
	         (16            << MAX2769_PLLIDR_RDIV);
	max2769_set(MAX2769_PLLIDR, pllidr );
	// CFDR
	cfdr = (256             << MAX2769_CFDR_L_CNT)   |
	       (1563            << MAX2769_CFDR_M_CNT)   |
	       (0               << MAX2769_CFDR_FCLKIN)  |
	       (1               << MAX2769_CFDR_ADCCLK)  |
	       (0               << MAX2769_CFDR_SERCLK)  |
	       (1               << MAX2769_CFDR_MODE);
	max2769_set(MAX2769_CFDR, cfdr );
}


uint16_t gpsbuf1[GPS_BUFFER_SIZE];
uint16_t gpsbuf2[GPS_BUFFER_SIZE];

static const MAX2769Config max2769 =
{
	.spi_sck     = {GPIOB, GPIOB_MAX_CFG_SCLK},
	.spi_mosi    = {GPIOB, GPIOB_MAX_CFG_MOSI},
	.spi_miso    = {GPIOB, GPIOB_MAX_CFG_MISO},
	.spi_cs      = {GPIOE, GPIOE_MAX_CFG_CS},
	.SPIDCONFIG  = &SPID2,
	.SPIDREAD    = &SPID1,
	.idle        = {GPIOE, GPIOE_MAX_IDLE},
	.shdn        = {GPIOE, GPIOE_MAX_SHDN},
	.q1_timesync = {GPIOE, GPIOE_TIMESYNC},
	.q0_datasync = {GPIOE, GPIOE_DATASYNC},
	.ld          = {GPIOB, GPIOB_LD},
	.antflag     = {GPIOB, GPIOB_ANTFLAG},
	.i1_clk_ser  = {GPIOA, GPIOA_CLK_SER},
	.i0_data_out = {GPIOB, GPIOB_DATA_OUT},
	.spi1_nss    = {GPIOA, GPIOA_PIN4},
	.bufs        = {gpsbuf1, gpsbuf2},
};


static void gps_handler(eventid_t id UNUSED){
	uint16_t *buf = max2769_getdata();
	ledToggle(&RED);
	max2769_donewithdata();
}

void main(void) {
	halInit();
	chSysInit();
	ledStart(NULL);

	max2769_init(&max2769);
	max2769_config();

	/* Manage MAX2769 events */
	struct EventListener ddone;
	static const evhandler_t evhndl[] = {
		gps_handler
	};
	chEvtRegister(&MAX2769_write_done, &ddone, 0);
	while(TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
