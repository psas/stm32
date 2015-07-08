/*! \file main.c
 *
 *   Experiment with the MAX2769 configuration
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "evtimer.h"

#include "net_addrs.h"
#include "rci.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "MAX2769.h"

#include "commands.h"

// Configuration from Google doc MAX2769RegisterConfiguration
static const uint32_t conf1 =
	(0b1            << MAX2769_CONF1_CHIPEN ) |
	(0b0            << MAX2769_CONF1_IDLE   ) |
	(0b1111         << MAX2769_CONF1_ILNA1  ) |
	(0b11           << MAX2769_CONF1_ILNA2  ) |
	(0b11           << MAX2769_CONF1_ILO    ) |
	(0b11           << MAX2769_CONF1_IMIX   ) |
	(0b0            << MAX2769_CONF1_MIXPOLE) |
	(0b01           << MAX2769_CONF1_LNAMODE) |
	(0b1            << MAX2769_CONF1_MIXEN  ) |
	(0b0            << MAX2769_CONF1_ANTEN  ) |
	(0b000000       << MAX2769_CONF1_FCEN   ) |
	(0b00           << MAX2769_CONF1_FBW    ) |
	(0b0            << MAX2769_CONF1_F3OR5  ) |
	(0b0            << MAX2769_CONF1_FCENX  ) |
	(0b1            << MAX2769_CONF1_FGAIN  );

static const uint32_t conf2 =
	(0b1            << MAX2769_CONF2_IQEN   ) |
	(0b10101010     << MAX2769_CONF2_GAINREF) |
	(0b01           << MAX2769_CONF2_AGCMODE) |
	(0b01           << MAX2769_CONF2_FORMAT ) |
	(0b010          << MAX2769_CONF2_BITS   ) |
	(0b00           << MAX2769_CONF2_DRVCFG ) |
	(0b1            << MAX2769_CONF2_LOEN   ) |
	(0b00           << MAX2769_CONF2_DIEID  );

static const uint32_t conf3 =
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
	(0b0            << MAX2769_CONF3_DATSYNCEN ) |
	(0b0            << MAX2769_CONF3_STRMRST   );

static const uint32_t pllconf =
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

static const uint32_t pllidr =
	(1540         << MAX2769_PLLIDR_NDIV) |
	(16           << MAX2769_PLLIDR_RDIV);

static const uint32_t cfdr =
	(1024         << MAX2769_CFDR_L_CNT ) |
	(1024         << MAX2769_CFDR_M_CNT ) |
	(0            << MAX2769_CFDR_FCLKIN) |
	(0            << MAX2769_CFDR_ADCCLK) |
	(1            << MAX2769_CFDR_SERCLK) |
	(0            << MAX2769_CFDR_MODE  );

static PWMConfig pwmcfg = {
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


#define SEQ_COUNTER_OFFSET 4

static uint8_t max2769_buf1[SEQ_COUNTER_OFFSET + GPS_BUFFER_SIZE];
static uint8_t max2769_buf2[SEQ_COUNTER_OFFSET + GPS_BUFFER_SIZE];

const MAX2769Config max2769 = {
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
	// GPS driver shouldn't know about the seqence counter section of the buffer
	.bufs = {max2769_buf1 + SEQ_COUNTER_OFFSET, max2769_buf2 + SEQ_COUNTER_OFFSET},
};

static int max2769_socket;
static int venus_socket;
#define VENUS_BUFFER_SIZE (500)
static uint8_t venus_buf1[SEQ_COUNTER_OFFSET + VENUS_BUFFER_SIZE];
static uint8_t venus_buf2[SEQ_COUNTER_OFFSET + VENUS_BUFFER_SIZE];
static uint8_t * venus_buf[2] = {venus_buf1, venus_buf2};
static uint8_t front = 0;
static EvTimer timer;

static void max2769_handler(eventid_t id UNUSED){
	static uint32_t seq_counter = 0;
	uint8_t *buf = max2769_getdata() - SEQ_COUNTER_OFFSET; //Add back in the sequence counter
	((uint32_t*)buf)[0] = seq_counter;
	write(max2769_socket, buf, SEQ_COUNTER_OFFSET + GPS_BUFFER_SIZE);
	++seq_counter;
	ledOn(&RED);
}


static void uart_error(UARTDriver *uartp UNUSED, uartflags_t e UNUSED) {
	ledOn(&RED);
}

static void venus_timeout(eventid_t id UNUSED) {
	static uint32_t seq_counter = 0;
	size_t not_received = uartStopReceive(&UARTD6);
	size_t received = VENUS_BUFFER_SIZE - not_received;
	if (received > 0) {
		((uint32_t*)(venus_buf[front]))[0] = seq_counter;
		write(venus_socket, venus_buf[front], SEQ_COUNTER_OFFSET + received);
		++seq_counter;
	}

	front = !front;
	//UART driver shouldn't have access to the sequence counter section of the buffer
	uartStartReceive(&UARTD6, VENUS_BUFFER_SIZE, venus_buf[front] + SEQ_COUNTER_OFFSET);
}


static UARTConfig venus = {
#ifndef FLIGHT
	.txend1_cb = txend,
#else
	.txend1_cb = NULL,
#endif
	.txend2_cb = NULL,
	.rxend_cb = NULL,
	.rxchar_cb = NULL,
	.rxerr_cb = uart_error,
	.speed = 115200,
	.cr1 = 0,
	.cr2 = 0,
	.cr3 = 0,
};

void main(void) {
	watchdogChibiosStart();

	ledStart(NULL);
	lwipThreadStart(GPS_LWIP);

	uartStart(&UARTD6, &venus);

	venus_socket = get_udp_socket(GPS_COTS_ADDR);
	chDbgAssert(venus_socket >= 0, "Venus socket failed", NULL);
	connect(venus_socket, FC_ADDR, sizeof(struct sockaddr));

	max2769_socket = get_udp_socket(GPS_OUT_ADDR);
	chDbgAssert(max2769_socket >= 0, "MAX2769 socket failed", NULL);
	connect(max2769_socket, FC_ADDR, sizeof(struct sockaddr));

	max2769_init(&max2769);
	max2769_set(MAX2769_CONF1, conf1);
	max2769_set(MAX2769_CONF2, conf2);
	max2769_set(MAX2769_CONF3, conf3);
	max2769_set(MAX2769_PLLCONF, pllconf);
	max2769_set(MAX2769_PLLIDR, pllidr);
	max2769_set(MAX2769_CFDR, cfdr);

	struct RCICommand commands[] = {
		RCI_CMD_VERS,
#ifndef FLIGHT
		RCI_CMD_CONF,
		RCI_CMD_DEBG,
		RCI_CMD_VNUS,
#endif
		{NULL}
	};
	RCICreate(commands);

	//UART driver shouldn't have access to the sequence counter section of the buffer
	uartStartReceive(&UARTD6, VENUS_BUFFER_SIZE, venus_buf[front] + SEQ_COUNTER_OFFSET);
	evtInit(&timer, MS2ST(10));
	evtStart(&timer);

	/* Manage GPS events */

	struct EventListener eltimer;
	struct EventListener ddone;

	chEvtRegister(&MAX2769_read_done, &ddone, 0);
	chEvtRegister(&timer.et_es, &eltimer, 1);

	static const evhandler_t evhndl[] = {
		max2769_handler,
		venus_timeout,
	};

	while(TRUE) {
		chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
	}
}


