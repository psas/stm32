#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "utils_led.h"
#include "utils_shell.h"
#include "usbdetail.h"

#include "MAX2769.h"

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

void main(void)
{
	halInit();
	chSysInit();
	ledStart(NULL);
	max2769_init(&max2769);
	const ShellCommand commands[] =
	{
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
	};
	usbSerialShellStart(commands);
#if MAX2769_SPI_DEBUG
	max2769_test_spi();
#endif
	/* Manage MAX2769 events */
	//struct EventListener ddone;
	static const evhandler_t evhndl[] =
	{
	};
	while(TRUE)
	{
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}

