#include <string.h>
#include "ch.h"
#include "lwip/sockets.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "rci.h"

static void handle_command(struct RCICmdData * data, struct RCICommand * cmd){
    for(;cmd->name != NULL; ++cmd){
        int rclen = strlen(cmd->name);
        if(!strncmp(data->cmd_data, cmd->name, MIN(data->cmd_len, rclen))){
            //remove rci command from cmd_data and place it in cmd_name
            data->cmd_name = cmd->name;
            data->cmd_len -= rclen;
            data->cmd_data += rclen;
            if(!data->cmd_len){
                data->cmd_data = NULL;
            }
            //call the callback
            if(cmd->function){
                cmd->function(data, cmd->user_data);
            }
            break;
        }
    }
}

static int get_command(int s, char * buffer, int buflen){
    int cmdlen = 0;
    int i = 0;
    struct fd_set read;
    FD_ZERO(&read);
    FD_SET(s, &read);
    do {
        struct timeval timeout = {5, 0};
        if(select(s+1, &read, NULL, NULL, &timeout) < 0){
            return -1;
        }
        int len = read(s, buffer, buflen-cmdlen);
        if(len < 0){
            return -1;
        }
        cmdlen += len;
        for(; i + 1 < cmdlen; ++i){
            if(buffer[i] == '\r' && buffer[i + 1] == '\n'){
                /* remove trailing \r\n */
                return i;
            }
        }
    } while(cmdlen < buflen);
    return -1;
}

WORKING_AREA(wa_rci, THD_WA_SIZE(2048 + ETH_MTU*2));
static msg_t rci_thread(void *p){
    chRegSetThreadName("RCI");

    struct RCICommand * commands = (struct RCICommand *)p;
    struct sockaddr from;
    socklen_t fromlen;

    struct sockaddr own;
    set_sockaddr(&own, "0.0.0.0", 23);

    char rx_buf[ETH_MTU];
    char tx_buf[ETH_MTU];

    int socket = socket(AF_INET, SOCK_STREAM, 0);
    chDbgAssert(socket >= 0, "Could not get RCI socket", NULL);
    if(bind(socket, &own, sizeof(struct sockaddr_in)) < 0){
        chDbgPanic("Could not bind RCI socket");
    }
    if(listen(socket, 1) < 0){
        chDbgPanic("Could not listen on RCI socket");
    }

    while(TRUE) {
        struct RCICmdData data = {
            .cmd_name = NULL,
            .cmd_data = rx_buf,
            .cmd_len = 0,
            .return_data = tx_buf,
            .return_len = 0,
        };

        fromlen = sizeof(from);
        int s = accept(socket, &from, &fromlen);
        if(s < 0){
            continue;
        }

        data.cmd_len = get_command(s, rx_buf, sizeof(rx_buf));
        if(data.cmd_len < 0){
            close(s);
            continue;
        }

        handle_command(&data, commands);

        //if there's data to return, return it to the address it came from
        data.return_len = MIN(data.return_len, ETH_MTU-2);
        if(data.return_len > 0){
            data.return_data[data.return_len] = '\r';
            data.return_data[data.return_len+1] = '\n';
            if(write(s, data.return_data, MIN(data.return_len + 2, ETH_MTU)) < 0){
                close(s);
                continue;
            }
        }
        close(s);
    }
    return -1;
}

void RCICreate(struct RCICommand * cmd){
    chDbgAssert(cmd, "RCICreate needs a config", NULL);

#if 0 //FIXME: because of threads this doesn't actually work all the time.
      //       What's a better way of finding lwip has started.
    /* Check if LWIP has been started */
    Thread * thd = chRegFirstThread();
    while(thd){
        if(!strcmp("lwipthread", thd->p_name)){
            break;
        }
        thd = chRegNextThread(thd);
    }
    chDbgAssert(thd, "RCICreate needs lwip started beforehand", NULL);
#endif
    chThdCreateStatic(wa_rci, sizeof(wa_rci), NORMALPRIO, rci_thread, (void *)cmd);
}


