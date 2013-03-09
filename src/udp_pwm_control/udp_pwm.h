/*function prototypes for udp stuff
Many thanks to http://www.backwoodsengineer.com/2013/02/geeky-post-stm32f4-bb-ethernet-projects.html
*/
void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
