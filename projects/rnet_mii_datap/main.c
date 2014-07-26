
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

#include "lwip/ip_addr.h"
#include "lwipopts.h"
#include "lwipthread.h"

#include "net_addrs.h"
#include "utils_led.h"
#include "utils_sockets.h"
#include "utils_shell.h"
#include "utils_general.h"

// Local header files

/*! The goal of this code is to run the shell through the serial terminal
 * and not the usb subsystem. Connect an FTDI serial/usb connector to the
 * appropriate (in rocketnet hub land this is pa9 and pa10)
 * These are configured as alternate functions in the board.h file.
 *
 * See the rocketnet-hub schematic for pinout.
 *
 *     stm		rnethub      ftdi
 * RX: pa10     ?            tx
 * TX: pa9      ?            rx
 *
 * In the mcuconf.h file enable the proper serial subsystem:
 * #define STM32_SERIAL_USE_USART1             TRUE
 *
 * In the halconf.h enable the serial system
 * #define HAL_USE_SERIAL                      TRUE
 */


#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

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

/*! \brief Assert or deassert reset GPIO_D4_ETH_N_RST
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_ksz_rst_n(BaseSequentialStream *chp, int argc, char *argv[]) {
	bool      pad_state;

	if(argc == 0) {
		// get current state of pin
		pad_state = palReadPad(GPIOD, GPIO_D4_ETH_N_RST);
		chprintf(chp, "GPIO_D4_ETH_N_RST:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "h", 1)  == 0)) {
		goto ERROR;
	}

	if ((argc == 1) && (strncmp(argv[0], "off", 3)  == 0)) {
		// deassert
		chprintf(chp, "deassert\r\n");
		palSetPad(GPIOD, GPIO_D4_ETH_N_RST);
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "on", 2) == 0)) {
		// assert
		chprintf(chp, "assert\r\n");
		palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
		return;
	}

	ERROR:
	chprintf(chp, "Usage: ksz_rst_n\r\n");
	chprintf(chp, "       ksz_rst_n <option>\r\n");
	chprintf(chp, "       where option may be 'on' or 'off'\r\n");
	chprintf(chp, "Command without option will return current setting of GPIO_D4_ETH_N_RST.\r\n");
	return;
}




/*! \brief Power on or off KSZ through GPIO_D14_KSZ_EN
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_ksz_pwr(BaseSequentialStream *chp, int argc, char *argv[]) {
	bool      pad_state;

	if(argc == 0) {
		// get current state of pin
		pad_state = palReadPad(GPIOD, GPIO_D14_KSZ_EN);
		chprintf(chp, "GPIO_D14_KSZ_EN:\t\t%s\r\n", pad_state ? "HIGH" : "LOW");
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "h", 1)  == 0)) {
		goto ERROR;
	}

	if ((argc == 1) && (strncmp(argv[0], "on", 2)  == 0)) {
		// set to on
		palSetPad(GPIOD, GPIO_D14_KSZ_EN);
		return;
	}

	if ((argc == 1) && (strncmp(argv[0], "off", 3) == 0)) {
		// set to off
		palClearPad(GPIOD, GPIO_D14_KSZ_EN);
		return;
	}

	ERROR:
	chprintf(chp, "Usage: ksz_pwr\r\n");
	chprintf(chp, "       ksz_pwr <option>\r\n");
	chprintf(chp, "       where option may be 'on' or 'off'\r\n");
	chprintf(chp, "Command without option will return current setting of GPIO_D14_KSZ_EN.");
	return;
}


/*! \brief Show reg
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_show(BaseSequentialStream *chp, int argc, char *argv[]) {
	//uint32_t i = 0;
	//MACDriver *macp = &ETHD1;

	(void)argv;
    (void)argc;

    chprintf(chp, "I am the Rocketnet Hub board.\r\n");

	chprintf(chp, "RCC->CFGR:\t0x%x\r\n",   RCC->CFGR);
	chprintf(chp, "SYSCFG->PMC:\t0x%x\r\n", SYSCFG->PMC);


//	chprintf(chp, "BOARD_PHY_ID>>16:\t0x%x\r\n", (BOARD_PHY_ID>>16));
//	chprintf(chp, "BOARD_PHY_ID&0xFFF0\t0x%x\r\n", (BOARD_PHY_ID & 0xFFF0));
//	chprintf(chp, "MACMIIDR_CR: 0x%x\tmacp->phyaddr: 0x%x\t\r\n",MACMIIDR_CR, macp->phyaddr);

//	for(i=0; i<32; ++i){
//		chprintf(chp, "i: %d\tphy1_g:\t0x%x\tphy2_g:\t0x%x\r\n", i, phy1_g[i], phy2_g[i]);
//	}


}

#define DATA_UDP_SEND_THREAD_STACK_SIZE      512
#define DATA_UDP_RECEIVE_THREAD_STACK_SIZE   512

#define DATA_UDP_RX_THD_PRIORITY             (LOWPRIO)
#define DATA_UDP_THREAD_PRIORITY             (LOWPRIO + 2)

#define DATA_UDP_MSG_SIZE                    50


WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_send_thread(void *p UNUSED) {
	chRegSetThreadName("data_udp_send_thread");

	int s = get_udp_socket(RNH_BATTERY_ADDR);
	if(s < 0){
		return -1;
	}
	connect(s, FC_ADDR, sizeof(struct sockaddr));

	char msg[DATA_UDP_MSG_SIZE] ;
	uint8_t count = 0;
	while (TRUE) {
		chsnprintf(msg, sizeof(msg), "rnet tx: %d \r\n", count++);
		write(s, msg, sizeof(msg));
		chThdSleepMilliseconds(500);
	}
	return -1;
}


WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_receive_thread(void *p UNUSED) {
	chRegSetThreadName("data_udp_receive_thread");

	BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;
	uint8_t count  = 0;
	uint8_t inbuf [DATA_UDP_MSG_SIZE + 1];

	int s = get_udp_socket(RNH_PORT_ADDR);
	if (s < 0) {
		return -1;
	}
	while(TRUE) {
		read(s, inbuf, DATA_UDP_MSG_SIZE);
		inbuf[DATA_UDP_MSG_SIZE] = '\0';
		chprintf(chp, "rnet rx:%d:->%s\r\n", count++, inbuf);
	}
	return -1;
}
static const ShellCommand commands[] = {
		{"ksz_rst_n", cmd_ksz_rst_n},
		{"ksz_pwr", cmd_ksz_pwr},
		{"show"    , cmd_show},
		{"mem"    , cmd_mem},
		{"threads", cmd_threads},
		{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
		(BaseSequentialStream *)&SD1,
		commands
};

void debug_msg_lwip(char* msg) {
    BaseSequentialStream *chp   =  (BaseSequentialStream *)&SD1;

    chprintf(chp, "%s\r\n", msg);
    chprintf(chp, "%d\r\n",  LWIP_SEND_TIMEOUT  );

}



void init_rnet(void) {
	palSetPad(GPIOD, GPIO_D14_KSZ_EN);
	palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
        palClearPad(GPIOD, GPIO_D14_KSZ_EN);
	chThdSleepMilliseconds(200);
	palSetPad(GPIOD, GPIO_D14_KSZ_EN);     // enable pwr
// palClearPad(GPIOD, GPIO_D13_RGB_R);
	chThdSleepMilliseconds(200);
	// Turn on clock from HSE -> PC9 function MCO2 see board file
	RCC->CFGR |=  (1<<31);      // MCO0
	RCC->CFGR &= ~(1<<29);

//	RCC->CFGR &=  ~(0b11<<22);      // MCO1
//	RCC->CFGR |=  (0b10<<21);
//
//	RCC->CFGR &= ~(0b111<<26); // clear MCO1 prescaler
//	RCC->CFGR |= (0b???<<26);  // set MCO1 prescaler
// Timer 1 channel one instead?

	palSetPad(GPIOD, GPIO_D4_ETH_N_RST);   // disable reset
// palClearPad(GPIOD, GPIO_D11_RGB_B);
	chThdSleepMilliseconds(500);
}

/*
 * Application entry point.
 */
void main(void) {
	static Thread            *shelltp       = NULL;
	halInit();
	chSysInit();

	ledStart(NULL);
	init_rnet();

	// start the serial port
	sdStart(&SD1, NULL);

	/*
	 * Shell manager initialization.
	 */
	shellInit();

	lwipThreadStart(RNH_LWIP);
	chThdCreateStatic(wa_data_udp_send_thread   , sizeof(wa_data_udp_send_thread)   , NORMALPRIO    , data_udp_send_thread   , NULL);
	chThdCreateStatic(wa_data_udp_receive_thread, sizeof(wa_data_udp_receive_thread), NORMALPRIO    , data_udp_receive_thread, NULL);


	while (true) {
		if (!shelltp )
			shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chThdSleepMilliseconds(1000);
	}
}

