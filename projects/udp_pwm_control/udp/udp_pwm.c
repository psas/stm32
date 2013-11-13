/*based heavily upon udp echo server example*/
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
/*#include "usb_cdc.h"*/
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <stdio.h>
#include "udp_pwm.h"
/*BaseSequentialStream *chp;*/
void udp_receive_server_init(void) {
	struct udp_pcb *upcb;
	err_t err;
	/*create the control block*/
	upcb = udp_new();
	if (upcb) {
		/*if we have the control block, attempt to bind to the port*/
		err = udp_bind(upcb, IP_ADDR_ANY, PWM_REC_PORT); /*IP_ADDR_ANY means just that.  UDP_SERVER_PORT should be defined in main */
		if (err == ERR_OK) {
			/*if we're good, we pass the callback function that handles the packet to udp_recv, which i assume is a blocking operation
			does chibios have blocking operations?  will Batman escape?*/
			udp_recv(upcb, udp_pwm_control_receive_callback, NULL);
		} else {
			/*couldn't bind to ip/port*/
		}
	} else {
		/*No control block for us*/
	}
}

void udp_pwm_control_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	/*need to figure out how to tear the packet apart and read from it*/
	chEvtBroadcast(&packet_event);
	printf("We got a packet!\n");
}
