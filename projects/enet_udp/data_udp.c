/*!
 * \file data_udp.c
 */

/*! \brief Experiment with sending sensor data over UDP connection.
 * \defgroup dataudp Data UDP PSAS Experiment
 * @{
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "usbdetail.h"
#include "utils_sockets.h"

#include "data_udp.h"

WORKING_AREA(wa_data_udp_send_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

msg_t data_udp_send_thread(void *p __attribute__ ((unused))){
    /*
     * This thread initializes a socket and then sends the message
     * "PSAS Rockets! <num>" every half second over UDP to a host
     * socket.
     */

    chRegSetThreadName("data_udp_send_thread");
    BaseSequentialStream *chp = getUsbStream();
    struct sockaddr_in self_addr;
    set_sockaddr(&self_addr, IP_DEVICE, DATA_UDP_TX_THREAD_PORT);
    int s = get_udp_socket((struct sockaddr*)&self_addr);

    //Create the address to send to
    struct sockaddr_in dest_addr;
    set_sockaddr(&dest_addr, IP_HOST, DATA_UDP_TX_THREAD_PORT);

    //send data to another socket
    char msg[DATA_UDP_MSG_SIZE];
    uint8_t count;
    for(count = 0;; ++count){
        chsnprintf(msg, sizeof(msg), "PSAS Rockets! %d", count);
        if(sendto(s, msg, sizeof(msg), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
            chprintf(chp, "Send socket send failure\r\t");
            return -3;
        }
        chThdSleepMilliseconds(500);
    }
}
/*!
 * Stack area for the data_udp_receive_thread.
 */
WORKING_AREA(wa_data_udp_receive_thread, DATA_UDP_SEND_THREAD_STACK_SIZE);

/*!
 * data_udp_rx  thread.
 */
msg_t data_udp_receive_thread(void *p __attribute__ ((unused))) {
    /*
     * This thread creates a UDP socket and then listens for any incomming
     * message, printing it out over serial USB
     */

    chRegSetThreadName("data_udp_receive_thread");
    BaseSequentialStream *chp = getUsbStream();
    struct sockaddr_in self_addr;
    set_sockaddr(&self_addr, IP_DEVICE, DATA_UDP_RX_THREAD_PORT);
    int s = get_udp_socket((struct sockaddr*)&self_addr);

    //read data from socket
    char msg[DATA_UDP_MSG_SIZE];
    while(TRUE) {
        //we can use recv here because we dont care where the packets came from
        //If we did, recvfrom() is the function to use
        if(recv(s, msg, sizeof(msg), 0) < 0){
            chprintf(chp, "Receive socket recv failure \r\n");
            return -3;
        } else {
            chprintf(chp, "%s\r\n", msg);
        }
    }
}

//! @}

