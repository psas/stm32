/*
 * RNet Command Interpreter - the RCI.
 *
 * This is somewhat similar to ChibiOS' various/shell but more precisely tuned
 * to our needs for a command interpreter that listens on ethernet.
 *
 */

#ifndef RNET_CMD_INTERP_H_
#define RNET_CMD_INTERP_H_

#include "lwip/ip_addr.h"

/* Contains all the RCI generated data passed to an RCI command handler.
 * You probably wont need to create one of these.
 */
struct RCICmdData{
    const char * cmd_name;  // Name of the command
    const char * cmd_data;        // Data segment of a received command
    int cmd_len;            // Length of valid cmd_data

    char * return_data;     // Output. Place data here to send to addr in from
    int return_len;         // length of data in return_data. Max length TODO
};

/* RCI command handler function type*/
typedef void (*rcicmd_t)(struct RCICmdData * rci_data, void * user_data);

/* Data type that ties an rcicmd name to a function */
struct RCICommand{
    const char *name;  //command name
    rcicmd_t function; //
    void * user_data;  //user data to pass to the function
};

/* Configuration for the RCI */
struct RCIConfig{
    const struct sockaddr *address;          // address of the port to listen on
    struct RCICommand *commands; // list of commands
};

/* Starts the RCI. Once an RCIConfig has been filled out, this will handle
 * setting up the socket and a thread to run the RCI in. The lwip main thread
 * is required to have been started beforehand.
 */
void RCICreate(struct RCIConfig * conf);

#endif /* RNET_CMD_INTERP_H_ */
