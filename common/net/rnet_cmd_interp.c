
#include <string.h>

#include "ch.h"
#include "lwip/sockets.h"

#include "device_net.h"

#include "rnet_cmd_interp.h"

static char rx_buf[500]; //fixme: this should be size of packet, 1400? also should be considered in the thread size magic number
static char tx_buf[500];

#define COMPARE_BUFFER_TO_CMD(a, alen, b, blen)\
    !strncmp(((char*)a), (b), blen > alen? blen: alen)

static void handle_command(struct RCICmdData * data, struct RCICommand * cmd){
    int rclen;
    for(;cmd->name != NULL; ++cmd){
        rclen = strlen(cmd->name);
        if(COMPARE_BUFFER_TO_CMD(data->cmd_data, data->cmd_len, cmd->name, rclen)){
            cmd->function(data, cmd->user_data);
            break;
        }
    }
}


WORKING_AREA(wa_rci, THD_WA_SIZE(2048)); //fixme: magic numbers
static msg_t rci_thread(void *p){
    struct RCIConfig * conf = (struct RCIConfig *)p;

    int socket = get_udp_socket(conf->address);
    int len;
    struct sockaddr from;
    socklen_t fromlen;

    struct RCICmdData data = {
        .cmd_data = rx_buf,
        .cmd_len = 0,
        .return_data = tx_buf,
        .return_len = 0,
        .from = &from,
        .fromlen = 0
    };
    //todo: restart sockets if they break
    while(TRUE) {
        len = recvfrom(socket, rx_buf, sizeof(rx_buf), 0, &from, &fromlen);
        if(len< 0){
            break;
        }
        data.cmd_len = len;
        data.fromlen = fromlen;
        handle_command(&data, conf->commands);
        if(data.return_len > 0){
            sendto(socket, data.return_data, data.return_len, 0, &from, fromlen);
        }
        data.return_len=0;
    }

    return -1;
}

void RCICreate(struct RCIConfig * conf){
    //todo: check if lwip is up.
    chThdCreateStatic(wa_rci, sizeof(wa_rci), NORMALPRIO, rci_thread, (void *)conf);
}


