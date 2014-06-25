#include <string.h>
#include "ch.h"
#include "lwip/sockets.h"
#include "utils_sockets.h"
#include "rnet_cmd_interp.h"

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

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

WORKING_AREA(wa_rci, THD_WA_SIZE(2048 + ETH_MTU*2));
static msg_t rci_thread(void *p){
    chRegSetThreadName("RCI");

    struct RCIConfig * conf = (struct RCIConfig *)p;
    struct sockaddr from;
    socklen_t fromlen;

    char rx_buf[ETH_MTU];
    char tx_buf[ETH_MTU];

    int socket = socket(AF_INET, SOCK_STREAM, 0);
    chDbgAssert(socket >= 0, "Could not get RCI socket", NULL);
    if(bind(socket, conf->address, sizeof(struct sockaddr_in)) < 0){
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
            return -1;
        }
        //Retrieve command
        do{
            int len = read(s, rx_buf + data.cmd_len, sizeof(rx_buf) - data.cmd_len);
            if(len < 0){
                return -1;
            }
            data.cmd_len += len;
        }while(data.cmd_len >=2 && rx_buf[data.cmd_len-2] !='\r' && rx_buf[data.cmd_len-1] != '\n');
        data.cmd_len -= 2;

        handle_command(&data, conf->commands);

        data.return_len = MIN(data.return_len, ETH_MTU-2);
        //if there's data to return, return it to the address it came from
        if(data.return_len > 0){
            data.return_data[data.return_len] = '\r';
            data.return_data[data.return_len+1] = '\n';
            if(write(s, data.return_data, MIN(data.return_len + 2, ETH_MTU)) < 0){
                return -1;
            }
        }
        close(s);
    }
    return -1;
}

void RCICreate(struct RCIConfig * conf){
    chDbgAssert(conf, "RCICreate needs a config", NULL);

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
    chThdCreateStatic(wa_rci, sizeof(wa_rci), NORMALPRIO, rci_thread, (void *)conf);
}


