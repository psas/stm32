/*
 * RNet Command Interpreter - the RCI.
 *
 * This is somewhat similar to ChibiOS' various/shell but more precisely tuned
 * to our needs for a command interpreter that listens on ethernet.
 */

#ifndef RNET_CMD_INTERP_H_
#define RNET_CMD_INTERP_H_

#include "lwip/ip_addr.h"

/* Contains all the RCI generated data passed to an RCI command handler.
 * You probably wont need to create one of these.
 */
struct RCICmdData{
	const char * name;  // Name of the command
	const char * data;  // Data segment of a received command
	int len;            // Length of valid cmd_data
};

struct RCIRetData{
	char * const data;  // Output. Place data here to send to addr in from
	int len;            // length of data in return_data. Max length is
                        // ETH_MTU from utils_socket.h
};

/* RCI command handler function type*/
typedef void (*rcicmd_t)(struct RCICmdData * cmd, struct RCIRetData * ret, void * user);

/* Data type that ties an rcicmd name to a function */
struct RCICommand{
	const char *name;  // Command name
	rcicmd_t function; // Command handler function to invoke
	void * user;       // User data to pass to the function
};

/* Starts the RCI. Once a null terminated RCICommand array has been filled out,
 * this will handle setting up the socket and a thread to run the RCI in. The
 * lwip main thread is required to have been started beforehand.
 *
 * The socket listens on port 23
 */
void RCICreate(struct RCICommand * cmd);

#endif /* RNET_CMD_INTERP_H_ */


