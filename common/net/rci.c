#include <string.h>
#include "ch.h"
#include "lwip/sockets.h"
#include "utils_general.h"
#include "utils_sockets.h"
#include "rci.h"

static void handle_command(
        struct RCICmdData * data,
        struct RCIRetData * ret,
        struct RCICommand * cmd)
{
    for(;cmd->name != NULL; ++cmd){
        if(cmd->function == NULL)
            break;
        int rclen = strlen(cmd->name);
        if(!strncmp(data->data, cmd->name, MIN(data->len, rclen))){
            //remove rci command from data and place it in name
            data->name = cmd->name;
            data->len -= rclen;
            data->data += rclen;
            //call the callback
            cmd->function(data, ret, cmd->user);
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
        if(select(s+1, &read, NULL, NULL, &timeout) <= 0){
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

static void send_response( int s, char * buffer, int len, int maxlen){
    // FIXME: check return of write, and continue to attempt to write if needed
    len = MIN(len, maxlen-2);
    if(len > 0){
        buffer[len] = '\r';
        buffer[len + 1] = '\n';
        if(write(s, buffer, MIN(len + 2, maxlen)) < 0){
            return;
        }
    }
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
        struct RCICmdData cmd = {
            .name = NULL,
            .data = rx_buf,
            .len = 0,
        };
        struct RCIRetData ret = {
            .data = tx_buf,
            .len = 0,
        };

        fromlen = sizeof(from);
        int s = accept(socket, &from, &fromlen);
        if(s < 0){
            continue;
        }

        cmd.len = get_command(s, rx_buf, sizeof(rx_buf));
        if(cmd.len < 0){
            close(s);
            continue;
        }

        handle_command(&cmd, &ret, commands);

        //if there's data to return, return it to the address it came from
        send_response(s, ret.data, ret.len, sizeof(tx_buf));
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


