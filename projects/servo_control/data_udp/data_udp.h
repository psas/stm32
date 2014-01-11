#ifndef _DATA_UDP_H
#define _DATA_UDP_H

/*
 * Includes
 * ======== ********************************************************************
 */

#include "lwip/ip_addr.h"
#include "psas_packet.h"


/*
 * Constant Definitions
 * ==================== ********************************************************
 */

#define DATA_UDP_TX_THREAD_STACK_SIZE 512
#define DATA_UDP_RX_THREAD_STACK_SIZE 512
#define DATA_UDP_RX_THREAD_PRIORITY   (LOWPRIO)
#define DATA_UDP_TX_THREAD_PRIORITY   (LOWPRIO + 2)
#define DATA_UDP_MSG_SIZE             50

#define PSAS_IP_BROADCAST ((uint32_t)0xffffffffUL)


/*
 * Declarations
 * ============ ****************************************************************
 */

extern WORKING_AREA(wa_data_udp_tx_thread, DATA_UDP_TX_THREAD_STACK_SIZE);
extern WORKING_AREA(wa_data_udp_rx_thread, DATA_UDP_RX_THREAD_STACK_SIZE);


// boilerplate
#ifdef __cplusplus
extern "C" {
#endif


msg_t data_udp_tx_launch_det(void *p);
msg_t data_udp_rx_thread(void *p);


// boilerplate
#ifdef __cplusplus
}
#endif
#endif
