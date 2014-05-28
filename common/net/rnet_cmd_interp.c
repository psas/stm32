#include <string.h>
#include "ch.h"
#include "hal.h"
#include "lwip/sockets.h"
#include "utils_sockets.h"
#include "rnet_cmd_interp.h"
#include "BQ3060.h"


#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)


EventSource BQ3060_battery_fault_hist;

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


//once issue 39 and 42 are resolved fold these into
//whatever command structure is used
static void BQ3060_getFaultData(void) {
	//event handler in projects/flight-rnh/main.c
	chEvtBroadcast(&BQ3060_battery_fault);
}

static void BQ3060_getFaultHistData(void) {
	chEvtBroadcast(&BQ3060_battery_fault_hist);

}

static void BQ3060_clearFaultHistory(void) {
	//clear fault history, see BQ3060.c
	cumAlarms[0] = 0;
	cumAlarms[1] = 0;
	cumAlarms[2] = 0;
}
WORKING_AREA(wa_rci, THD_WA_SIZE(2048 + ETH_MTU*2));
static msg_t rci_thread(void *p){
    chRegSetThreadName("RCI");

    struct RCIConfig * conf = (struct RCIConfig *)p;
    int socket = get_udp_socket(conf->address);
    struct sockaddr from;
    socklen_t fromlen;

    char rx_buf[ETH_MTU];
    char tx_buf[ETH_MTU];

    if(socket < 0){
        return -1;
    }

    while(TRUE) {

        struct RCICmdData data = {
            .cmd_name = NULL,
            .cmd_data = rx_buf,
            .cmd_len = 0,
            .return_data = tx_buf,
            .return_len = 0,
        };
        //recvfrom because we might return data
        data.cmd_len = recvfrom(socket, rx_buf, sizeof(rx_buf), 0, &from, &fromlen);
        if(data.cmd_len < 0){
            break;
        }

        handle_command(&data, conf->commands);

        //if there's data to return, return it to the address it came from
        if(data.return_len > 0){
            if(sendto(socket, data.return_data, MAX(data.return_len, ETH_MTU), 0, &from, fromlen) < 0){
                break;
            }
        }
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


