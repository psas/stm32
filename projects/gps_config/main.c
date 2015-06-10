/*! \file main.c
 *
 *   Experiment with the MAX2769 configuration
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "hal.h"

#include "net_addrs.h"
#include "rci.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "MAX2769.h"

static void max2769_config(void) {
	// Configuration from Google doc MAX2769RegisterConfiguration
	uint32_t conf1 =
		(0b1            << MAX2769_CONF1_CHIPEN ) |
		(0b0            << MAX2769_CONF1_IDLE   ) |
		(0b1111         << MAX2769_CONF1_ILNA1  ) |
		(0b00           << MAX2769_CONF1_ILNA2  ) |
		(0b11           << MAX2769_CONF1_ILO    ) |
		(0b11           << MAX2769_CONF1_IMIX   ) |
		(0b0            << MAX2769_CONF1_MIXPOLE) |
		(0b10           << MAX2769_CONF1_LNAMODE) |
		(0b1            << MAX2769_CONF1_MIXEN  ) |
		(0b0            << MAX2769_CONF1_ANTEN  ) |
		(0b000000       << MAX2769_CONF1_FCEN   ) |
		(0b00           << MAX2769_CONF1_FBW    ) |
		(0b0            << MAX2769_CONF1_F3OR5  ) |
		(0b0            << MAX2769_CONF1_FCENX  ) |
		(0b1            << MAX2769_CONF1_FGAIN  );

	uint32_t conf2 =
		(0b1            << MAX2769_CONF2_IQEN   ) |
		(0b10101010     << MAX2769_CONF2_GAINREF) |
		(0b01           << MAX2769_CONF2_AGCMODE) |
		(0b01           << MAX2769_CONF2_FORMAT ) |
		(0b010          << MAX2769_CONF2_BITS   ) |
		(0b00           << MAX2769_CONF2_DRVCFG ) |
		(0b1            << MAX2769_CONF2_LOEN   ) |
		(0b00           << MAX2769_CONF2_DIEID  );

	uint32_t conf3 =
		MAX2769_CONF3_RESERVED |
		(0b111010       << MAX2769_CONF3_GAININ    ) |
		(0b1            << MAX2769_CONF3_FSLOWEN   ) |
		(0b0            << MAX2769_CONF3_HILOADEN  ) |
		(0b1            << MAX2769_CONF3_ADCEN     ) |
		(0b1            << MAX2769_CONF3_DRVEN     ) |
		(0b1            << MAX2769_CONF3_FOFSTEN   ) |
		(0b1            << MAX2769_CONF3_FILTEN    ) |
		(0b0            << MAX2769_CONF3_FHIPEN    ) |
		(0b1            << MAX2769_CONF3_PGAIEN    ) |
		(0b1            << MAX2769_CONF3_PGAQEN    ) |
		(0b0            << MAX2769_CONF3_STRMEN    ) |
		(0b0            << MAX2769_CONF3_STRMSTART ) |
		(0b0            << MAX2769_CONF3_STRMSTOP  ) |
		(0b111          << MAX2769_CONF3_STRMCOUNT ) |
		(0b11           << MAX2769_CONF3_STRMBITS  ) |
		(0b0            << MAX2769_CONF3_STAMPEN   ) |
		(0b0            << MAX2769_CONF3_TIMESYNCEN) |
		(0b0            << MAX2769_CONF3_DATSYNCEN) |
		(0b0            << MAX2769_CONF3_STRMRST   );

	uint32_t pllconf =
		MAX2769_PLLCONF_RESERVED |
		(0b1          << MAX2769_PLL_VCOEN   ) |
		(0b0          << MAX2769_PLL_IVCO    ) |
		(0b1          << MAX2769_PLL_REFOUTEN) |
		(0b01         << MAX2769_PLL_REFDIV  ) |
		(0b01         << MAX2769_PLL_IXTAL   ) |
		(0b10000      << MAX2769_PLL_XTALCAP ) |
		(0b0000       << MAX2769_PLL_LDMUX   ) |
		(0b1          << MAX2769_PLL_ICP     ) |
		(0b0          << MAX2769_PLL_PFDEN   ) |
		(0b000        << MAX2769_PLL_CPTEST  ) |
		(0b1          << MAX2769_PLL_INT_PLL ) |
		(0b0          << MAX2769_PLL_PWRSAV  );

	uint32_t pllidr =
		(1540         << MAX2769_PLLIDR_NDIV) |
		(16           << MAX2769_PLLIDR_RDIV);

	uint32_t cfdr =
		(1024         << MAX2769_CFDR_L_CNT ) |
		(1024         << MAX2769_CFDR_M_CNT ) |
		(0            << MAX2769_CFDR_FCLKIN) |
		(0            << MAX2769_CFDR_ADCCLK) |
		(1            << MAX2769_CFDR_SERCLK) |
		(0            << MAX2769_CFDR_MODE  );

	max2769_set(MAX2769_CONF1, conf1);
	max2769_set(MAX2769_CONF2, conf2);
	max2769_set(MAX2769_CONF3, conf3);
	max2769_set(MAX2769_PLLCONF, pllconf);
	max2769_set(MAX2769_PLLIDR, pllidr);
	max2769_set(MAX2769_CFDR, cfdr);
}



uint8_t gpsbuf1[4+GPS_BUFFER_SIZE];
uint8_t gpsbuf2[4+GPS_BUFFER_SIZE];

PWMConfig pwmcfg = {
	.frequency = 84000000,
	.period = 2,
	.callback = NULL,
	.channels = {
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
	},
	/* STM32 Specific config options */
	.cr2 = 0,
	.dier = 0
};

static const MAX2769Config max2769 = {
	.max = {
		.SPID    = &SPID2,
		.sck     = {GPIOB, GPIOB_MAX_CFG_SCLK},
		.mosi    = {GPIOB, GPIOB_MAX_CFG_MOSI},
		.nss     = {GPIOE, GPIOE_MAX_CFG_CS},
		.idle    = {GPIOE, GPIOE_MAX_IDLE},
		.shdn    = {GPIOE, GPIOE_MAX_SHDN},
		.ld      = {GPIOB, GPIOB_LD},
		.antflag = {GPIOB, GPIOB_ANTFLAG},
	},
	.cpld = {
		.SPID    = &SPID1,
		.mosi    = {GPIOB, GPIOB_CPLD_OUT_SPI1_MOSI},
		.sck     = {GPIOA, GPIOA_CPLD_OUT_SPI1_SCK},
		.nss     = {GPIOA, GPIOA_CPLD_OUT_SPI1_NSS},
		.clk_src = {GPIOB, GPIOB_CPLD_OUT_SPI_CLK_SRC},
		.reset   = {GPIOA, GPIOA_CPLD_RESET},
		.debug   = {GPIOD, GPIOD_CPLD_DEBUG},
		.clk_src_cfg = &pwmcfg,
		.PWMD = &PWMD12,
	},
	.bufs = {gpsbuf1+4, gpsbuf2+4},
};

int gps_socket;
static void gps_handler(eventid_t id UNUSED){
	static uint32_t i = 0;
	if(i % 100 == 0) {
		ledToggle(&RED);
	}
	uint8_t *buf = max2769_getdata();
	buf = buf - 4;
	((uint32_t*)buf)[0] = i;
	write(gps_socket, buf, 4+GPS_BUFFER_SIZE);
	++i;
}

void setconf(struct RCICmdData * cmd, struct RCIRetData * ret UNUSED, void * user UNUSED) {
	if(cmd->len != 8) {
		return;
	}
	char regstr[2] = {cmd->data[0], '\0'};
	int reg  = strtol(regstr, NULL, 16);
	int data = strtol(cmd->data+1, NULL, 16);

	max2769_set(reg, data);
}

struct RCICommand RCI_CMD_CONF = {
	.name = "#CONF",
	.function = setconf,
	.user = NULL
};

void main(void) {
	halInit();
	chSysInit();
	ledStart(NULL);

	lwipThreadStart(GPS_LWIP);
	gps_socket = get_udp_socket(GPS_OUT_ADDR);
	connect(gps_socket, FC_ADDR, sizeof(struct sockaddr));
	max2769_init(&max2769);
	max2769_config();

	struct RCICommand commands[] = {
		RCI_CMD_VERS,
		RCI_CMD_CONF
	};
	RCICreate(commands);

	/* Manage MAX2769 events */
	struct EventListener ddone;
	static const evhandler_t evhndl[] = {
		gps_handler
	};
	chEvtRegister(&MAX2769_read_done, &ddone, 0);
	while(TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}


