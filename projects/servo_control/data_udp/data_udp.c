// stdblib
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "math.h"

// ChibiOS
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/ip_addr.h"

// PSAS
#include "device_net.h"
#include "data_udp.h"
#include "fc_net.h"
#include "pwm_config.h"
#include "psas_packet.h"
#include "usbdetail.h"



/*
 * Constant Definitions
 * ==================== ********************************************************
 */

// This enum corresponds to the PWM bisable bit in the message received from the
// flight computer. Since it's a _disable_ bit, when it is zero then the PWM is
// enabled.
enum {
	PWM_ENABLE = 0,
	PWM_DISABLE
};



/*
 * Global Variables
 * ================ ************************************************************
 */

// these are the stacks for the two UDP TX/RX threads defined in this file.
WORKING_AREA(wa_data_udp_tx_thread, DATA_UDP_TX_THREAD_STACK_SIZE);
WORKING_AREA(wa_data_udp_rx_thread, DATA_UDP_RX_THREAD_STACK_SIZE);



/*
 * UDP Send & Receive Threads
 * ========================== **************************************************
 */

/*
 * This thread appears to just set up a UDP connection, send out a Roll Control
 * message with the launch_detect field set to the current value of the pointer
 * passed as its sole argument (cast to bool), then tear down the connection and
 * return a RDY_OK msg. If anything goes wrong with the network bits, it tears
 * down the connection and returns a RDY_RESET msg.
 */
msg_t data_udp_tx_launch_det(void* launch_detect_voidp) {
    bool* launch_detect;
    launch_detect = (bool*) launch_detect_voidp;

    err_t           net_err;
    ip_addr_t       our_address;
    ip_addr_t       fc_address;
    struct netconn *conn;
    struct netbuf  *buff;
    char           *data;
    LaunchDetect   msg;

    ROLL_CTL_IP_ADDR(&our_address);
    IP_PSAS_FC(&fc_address);

    conn = netconn_new(NETCONN_UDP);
    LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);

    memset(&msg, 0, sizeof(LaunchDetect));

    /* Bind to the local address, or to ANY address */
    net_err = netconn_bind(conn, &our_address, ROLL_CTL_TX_PORT ); //local port

    // bail if there was an error binding to the address
    if (net_err != ERR_OK) {
        netconn_delete(conn);
        return RDY_RESET;
    }

    // Connect to fc specific/broadcast address.
    // TODO: Understand why a UDP needs a connect... this may be a LwIP thing
    // that chooses between tcp_/udp_/raw_ connections internally.
    net_err = netconn_connect(conn, &fc_address, FC_LISTEN_PORT);

    // bail if we couldn't connect to the flight computer's address
    if (net_err != ERR_OK) {
        netconn_delete(conn);
        return RDY_RESET;
    }

    buff = netbuf_new();
    data = netbuf_alloc(buff, sizeof(LaunchDetect));

    msg.launch_detect = (*launch_detect ? 1 : 0);
    memcpy(data, &msg, sizeof(LaunchDetect));

    netconn_send(conn, buff);

    netbuf_delete(buff); // De-allocate packet buffer
    netconn_delete(conn);
    return RDY_OK;
}


/*
 * This is the inner loop of the data_udp_rx_thread, wherein we read a Roll
 * Control message from the network and adjust the PWM output according to its
 * instructions.
 */
static void read_roll_ctl_and_adjust_pwm(struct netconn *conn) {
	RCOutput       rc_packet;
	err_t          net_err;
	struct netbuf *net_buff;
	u16_t          buff_len;
	char          *data;

	/*
	 * Read the data from the port, blocking if nothing yet there.
	 * We assume the request (the part we care about) is in one netbuf.
	 */
	net_err = netconn_recv(conn, &net_buff);

    // bail if there was an error reading the data in
    if (net_err != ERR_OK) {
        netconn_close(conn);
        netbuf_delete(net_buff); // netconn_recv assigns us ownership
        return;
    }

    netbuf_data(net_buff, (void **)&data, &buff_len);
    memcpy(&rc_packet, data, sizeof(RCOutput));

    if(rc_packet.u8ServoDisableFlag == PWM_ENABLE) {
        uint16_t width = rc_packet.u16ServoPulseWidthBin14;
        double   ms_d  = width/pow(2,14);
        double   us_d  = ms_d * 1000;

        pwm_set_pulse_width_ticks(pwm_us_to_ticks(us_d));
    } else {
        pwmDisableChannel(&PWMD4, 3);
    }

	/*
     * Delete the buffer (netconn_recv gives us ownership,
	 * so we have to make sure to deallocate the buffer)
	 */
    netconn_close(conn);
    netbuf_delete(net_buff); // netconn_recv assigns us ownership
}


/*!
 * This is the other part of the data_udp_rx_thread, which wraps the
 * read_roll_ctl_and_adjust_pwm routine above. It sets up the UDP connection,
 * bailing with a RDY_RESET msg if binding doesn't succeed; otherwise it loops
 * the above routine forever.
 */
msg_t data_udp_rx_thread(void *_) {
    _;

	err_t           net_err;
	struct netconn *conn;
	ip_addr_t       our_address;

	ROLL_CTL_IP_ADDR(&our_address);

	chRegSetThreadName("data_udp_receive_thread");

	// Create a new UDP connection handle.
	conn = netconn_new(NETCONN_UDP);
	LWIP_ERROR("data_udp_receive_thread: invalid conn", (conn != NULL), return RDY_RESET;);

	// Bind sensor address to a udp port
	net_err = netconn_bind(conn, &our_address, ROLL_CTL_LISTEN_PORT);

    // bail if we couldn't bind to our defined address & port
    if (net_err != ERR_OK) {
        return RDY_RESET;
    }

    while (1) {
        read_roll_ctl_and_adjust_pwm(conn);
    }

    // we should never get here but whatever
    return RDY_OK;
}
