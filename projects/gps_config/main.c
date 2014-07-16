/*! \file main.c
 *
 *   Experiment with the MAX2769 configuration
 */

#include <stddef.h>
#include "ch.h"
#include "hal.h"

#include "eventlogger.h"
#include "chprintf.h"
#include "utils_led.h"
#include "utils_shell.h"
#include "usbdetail.h"

#include "psas_rtc.h"
#include "psas_sdclog.h"

#include "MAX2769.h"
static 	int conf3_state       =  MAX2769_CONF3_DEF;

static void     max2769_config(void)
{
	BaseSequentialStream * chp;
	chp = getUsbStream();
	int conf1       =  0;
	int conf2       =  0;
	int conf3       =  0;
	int pllconf     =  0;
	int pllidr      =  0;
	int cfdr        =  0;
	//chprintf(chp, "Reset MAX2769...\r\n");
	max2769_reset();
	//chprintf(chp, "Start configuration...\r\n");
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
	chprintf(chp, "\tCONF1:\t\t0x%x\r\n", conf1);
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
	chprintf(chp, "\tCONF2:\t\t0x%x\r\n", conf2);
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
	        (0b0            << MAX2769_CONF3_STRMSTART   ) |
	        (0b0            << MAX2769_CONF3_STRMSTOP    ) |
	        (0b000          << MAX2769_CONF3_STRMCOUNT   ) |
	        (0b11           << MAX2769_CONF3_STRMBITS    ) |
	        (0b0            << MAX2769_CONF3_STRMPEN     ) |
	        (0b0            << MAX2769_CONF3_TIMESYNCEN  ) |
	        (0b0            << MAX2769_CONF3_DATASYNCEN  ) |
	        (0b0            << MAX2769_CONF3_STRMRST     );

	max2769_set(MAX2769_CONF3, conf3 );
	conf3_state = conf3;
	chprintf(chp, "\tCONF3:\t\t0x%x\r\n", conf3);

	// clear conf3-STRMSTART
	/*conf3 &= ~(0b1          << MAX2769_CONF3_STRMSTART   );*/
	/*set conf3-STRMSTART*/
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
	chprintf(chp, "\tPLLCONF:\t0x%x\r\n", pllconf);
	// PLLIDR
	pllidr = (1536          << MAX2769_PLLIDR_NDIV) |
	         (16            << MAX2769_PLLIDR_RDIV);
	max2769_set(MAX2769_PLLIDR, pllidr );
	chprintf(chp, "\tPLLIDR:\t\t0x%x\r\n", pllidr);
	// CFDR
	cfdr = (256             << MAX2769_CFDR_L_CNT)   |
	       (1563            << MAX2769_CFDR_M_CNT)   |
	       (0               << MAX2769_CFDR_FCLKIN)  |
	       (0               << MAX2769_CFDR_ADCCLK)  |
	       (1               << MAX2769_CFDR_SERCLK)  |
	       (1               << MAX2769_CFDR_MODE);
	max2769_set(MAX2769_CFDR, pllidr );
	chprintf(chp, "\tCFDR:\t\t0x%x\r\n", cfdr);
	//chprintf(chp, "End configuration...\r\n");
}

static void max2769_streamstart(void) {
	BaseSequentialStream * chp;
	chp = getUsbStream();
	int conf3       =  0;

	conf3 = conf3_state |  (0b1 << MAX2769_CONF3_STRMSTART | 1 << MAX2769_CONF3_DATASYNCEN | 1 << MAX2769_CONF3_TIMESYNCEN );
	max2769_set(MAX2769_CONF3, conf3 );

	conf3_state = conf3;
	chprintf(chp, "\tstart CONF3:\t\t0x%x\r\n", conf3);
}

static void max2769_streamstop(void) {
	BaseSequentialStream * chp;
	chp = getUsbStream();
	int conf3       =  0;

	conf3 = conf3_state |  (0b1 << MAX2769_CONF3_STRMSTOP );
	max2769_set(MAX2769_CONF3, conf3 );

	chprintf(chp, "\tstop CONF3:\t\t0x%x\r\n", conf3);
}

/*! \brief Configure max
 *
 * @param chp
 * @param argc
 * @param argv
 */
static void cmd_mconfig(BaseSequentialStream * chp, int argc, char * argv[])
{
	(void)chp;
	(void)argv;
	if (argc > 0)
	{
		chprintf(chp, "Usage: configure\r\n");
		return;
	}
	max2769_config();
}

static void cmd_msstart(BaseSequentialStream * chp, int argc, char * argv[])
{
	(void)chp;
	(void)argv;
	if (argc > 0)
	{
		chprintf(chp, "Usage: msstart\r\n");
		return;
	}
	max2769_streamstart();
}

static void cmd_msstop(BaseSequentialStream * chp, int argc, char * argv[])
{
	(void)chp;
	(void)argv;
	if (argc > 0)
	{
		chprintf(chp, "Usage: msstop\r\n");
		return;
	}
	max2769_streamstop();
}

void main(void)
{

    struct EventListener insertion_listener, removal_listener;

	halInit();
	chSysInit();

	psas_rtc_lld_init();
	ledStart(NULL);

	/*! \brief GPS
	  */
	max2769_init(&max2769_gps);

	/*! \brief power to sd card is controlled by N transistor on PD0
	  */
	palClearPad(GPIOD, GPIOD_SD_VDD);

    /*!
     * Activates the SDC driver 1 using default
     * configuration.
     */
    sdcStart(&SDCD1, NULL);

    /*!
     * Activates SD card insertion monitor & registers SD card events.
     */
    sdc_tmr_init(&SDCD1);
    chEvtRegister(&sdc_inserted_event, &insertion_listener, 0);
    chEvtRegister(&sdc_removed_event,  &removal_listener,   1);

	/*!
	 * Starts the eventlogger
	 */
	eventlogger_init();

	const ShellCommand commands[] =
	{
		{"mem",     cmd_mem},
		{"threads", cmd_threads},
		{"mconfig", cmd_mconfig},
		{"msstart", cmd_msstart},
		{"msstop",  cmd_msstop},
		{NULL, NULL}
	};

	usbSerialShellStart(commands);
	//max2769_config();
	/* Manage MAX2769 events */
	//struct EventListener ddone;
	static const evhandler_t evhndl[] =
	{
		sdc_insert_handler,
		sdc_remove_handler
	};

    // It is possible the card is already inserted. Check now by calling insert handler directly.
    sdc_insert_handler(0);

	while(TRUE)
	{
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}
