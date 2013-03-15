/*based heavily upon udp echo server example*/
void udp_receive_server_init(void) {
	struct udp_pcp *upcb;
	err_t err;
	/*create the control block*/
	upcb = udp_new();
	if (upcb) {
		/*if we have the control block, attempt to bind to the port*/
		err = udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT); /*IP_ADDR_ANY means just that.  UDP_SERVER_PORT should be defined in main */
		if (err == ERR_OK) {
			/*if we're good, we pass the callback function that handles the packet to udp_recv, which i assume is a blocking operation
			does chibios have blocking operations?  will Batman escape?*/
			udp_recv(upcb, udp_pwm_control_receive_callback, NULL);
		} else {
			/*couldn't bind to ip/port*/
			printf("Unable to bind to ip/port\n");
		}
	} else {
		/*No control block for us*/
		printf("Could not create UDP control block.\n");
	}
}

void udp_pwm_control_receive_callback (void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	/*need to figure out how to tear the packet apart and read from it*/
	printf("We got a packet!\n");
}
