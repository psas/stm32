/*function prototypes for udp stuff
Many thanks to http://www.backwoodsengineer.com/2013/02/geeky-post-stm32f4-bb-ethernet-projects.html
*/
#define PWM_REC_STACK_SIZE 1024
#define PWM_REC_PORT 4000
void udp_receive_server_init(void); 
void udp_pwm_control_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

