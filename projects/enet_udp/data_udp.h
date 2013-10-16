
/*!
 * \file data_udp.h
 */

 /* \brief UDP Server wrapper thread
 * @addtogroup dataudp
 * @{
 */

#ifndef _DATA_UDP_H
#define _DATA_UDP_H

#define DATA_UDP_SEND_THREAD_STACK_SIZE      512
#define DATA_UDP_RECEIVE_THREAD_STACK_SIZE   512

#define DATA_UDP_TX_THREAD_PORT              35000
#define DATA_UDP_RX_THREAD_PORT              35003

#define DATA_UDP_RX_THD_PRIORITY             (LOWPRIO)
#define DATA_UDP_THREAD_PRIORITY             (LOWPRIO + 2)

#define IP_PSAS_FC                           "192.168.0.91"
#define IP_PSAS_SENSOR                       "192.168.0.196"

#define DATA_UDP_MSG_SIZE                    50

extern WORKING_AREA(wa_data_udp_send_thread,    DATA_UDP_SEND_THREAD_STACK_SIZE);
extern WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_RECEIVE_THREAD_STACK_SIZE);

#ifdef __cplusplus
extern "C" {
#endif
  msg_t data_udp_send_thread(void *p);
  msg_t data_udp_receive_thread(void *p);

#ifdef __cplusplus
}
#endif

#endif

//! @}

