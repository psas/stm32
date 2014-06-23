/*! \file main.c
 *
 *   Experiment with the MAX2769 configuration
 */

#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "utils_led.h"
#include "utils_shell.h"
#include "usbdetail.h"

#include "MAX2769.h"

static void     max2769_config(void)
{
	BaseSequentialStream * chp;
	chp = getUsbStream();
	int conf1       = MAX2769_CONF1_DEF;
	int conf2       = MAX2769_CONF2_DEF;
	int conf3       = MAX2769_CONF3_DEF;
	int pllconf     = MAX2769_PLLCONF_DEF;
	int pllidr      = MAX2769_PLLIDR_DEF;
	int cfdr        = MAX2769_CFDR_DEF;

	chprintf(chp, "Reset MAX2769...\r\n");
	max2769_reset();

	chprintf(chp, "Start configuration...\r\n");

	conf1 = 0xbcf51a3;
	max2769_set(MAX2769_CONF1, conf1 );
	chprintf(chp, "\tCONF1: 0x%x\r\n", conf1);

	conf2 = 0x550288;
	max2769_set(MAX2769_CONF2, conf2 );
	chprintf(chp, "\tCONF2: 0x%x\r\n", conf2);

	conf3 = 0xeaff83a;
	max2769_set(MAX2769_CONF3, conf3 );
	chprintf(chp, "\tCONF3: 0x%x\r\n", conf3);

	pllconf = 0x9ac0008;
	max2769_set(MAX2769_PLLCONF, pllconf );
	chprintf(chp, "\tPLLCONF: 0x%x\r\n", pllconf);

	pllidr = (1536 << MAX2769_PLLIDR_NDIV) | (16 << MAX2769_PLLIDR_RDIV);
	max2769_set(MAX2769_PLLIDR, pllidr );
	chprintf(chp, "\tPLLIDR: 0x%x\r\n", pllidr);

	cfdr = (256 << MAX2769_CFDR_L_CNT) | (1563 << MAX2769_CFDR_M_CNT) |
	       (0 << MAX2769_CFDR_FCLKIN) | (0 << MAX2769_CFDR_ADCCLK) | (1 << MAX2769_CFDR_SERCLK) |
	       (1 << MAX2769_CFDR_MODE);
	max2769_set(MAX2769_CFDR, pllidr );
	chprintf(chp, "\tCFDR: 0x%x\r\n", cfdr);

	chprintf(chp, "End configuration...\r\n");
}


/*! \brief Configure max
 *
 * @param chp
 * @param argc
 * @param argv
 */
static void cmd_configure(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)chp;
    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: configure\r\n");
        return;
    }
    max2769_config();
}

void main(void)
{
	halInit();
	chSysInit();
	ledStart(NULL);
	max2769_init(&max2769_gps);
	const ShellCommand commands[] =
	{
		{"mem", cmd_mem},
		{"threads", cmd_threads},
		{"maxconfig", cmd_configure},
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

