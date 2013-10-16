/*
 */
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "KS8999.h"
#include "rnh_shell.h"

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
    static const char *states[] = {THD_STATE_NAMES};
    Thread *tp;

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: threads\r\n");
        return;
    }
    chprintf(chp, "addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");
    tp = chRegFirstThread();
    do {
        chprintf(chp, "%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n",
                (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
                (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
        tp = chRegNextThread(tp);
    } while (tp != NULL);
}

static void cmd_power(BaseSequentialStream *chp, int argc, char *argv[]) {
    // TODO: port aliases?
    if(argc < 1){
        chprintf(chp, "Usage: pwr <port ...> [on|off]\r\n");
        return;
    }

#define NUM_PORT 8
    int port[NUM_PORT] = {0, 0, 0, 0, 0, 0, 0, 0};
    //power and fault 0 are left at a safe value instead of NULL or similar
    uint32_t power[NUM_PORT] =
        {GPIO_E4_NC, GPIO_E0_NODE1_N_EN, GPIO_E1_NODE2_N_EN,
         GPIO_E2_NODE3_N_EN, GPIO_E3_NODE4_N_EN, GPIO_E4_NC,
         GPIO_E5_NODE6_N_EN, GPIO_E6_NODE7_N_EN};

    uint32_t fault[NUM_PORT] =
        {GPIO_E12_NC, GPIO_E8_NODE1_N_FLT, GPIO_E9_NODE2_N_FLT,
         GPIO_E10_NODE3_N_FLT, GPIO_E11_NODE4_N_FLT, GPIO_E12_NC,
         GPIO_E13_NODE6_N_FLT, GPIO_E14_NODE7_N_FLT};

    typedef enum {info, on, off} action;
    action act = info;
    //parse arguments
    int i;
    for(i = 0; i < argc; ++i){
        if(!strcmp(argv[i], "1")){
            port[1] = TRUE;
        }else if(!strcmp(argv[i], "2")){
            port[2] = TRUE;
        }else if(!strcmp(argv[i], "3")){
            port[3] = TRUE;
        }else if(!strcmp(argv[i], "4")){
            port[4] = TRUE;
        }else if(!strcmp(argv[i], "6")){
            port[6] = TRUE;
        }else if(!strcmp(argv[i], "7")){
            port[7] = TRUE;
        }else if(!strcmp(argv[i], "all")){
            port[1] = TRUE;
            port[2] = TRUE;
            port[3] = TRUE;
            port[4] = TRUE;
            port[6] = TRUE;
            port[7] = TRUE;
        }else if(i != 0 && i == argc - 1) {
            if(!strcmp(argv[i], "on")){
                act = on;
            }else if(!strcmp(argv[i], "off")){
                act = off;
            }else{
                chprintf(chp, "Invalid action: %s. Action must be on|off\r\n", argv[i]);
                return;
            }
        }else{
            chprintf(chp, "Invalid port: %s. Valid ports are 1, 2, 3, 4, 6, 7, all\r\n", argv[i]);
            return;
        }
    }

    //do actions
    for(i = 1; i < NUM_PORT; ++i){
        if(port[i]){
            switch(act){
            case on:
                palClearPad(GPIOE, power[i]);
                break;
            case off:
                palSetPad(GPIOE, power[i]);
                break;
            case info:
            default:
                chprintf(chp, "%d: ", i);
                if(palReadPad(GPIOE, power[i])){
                    chprintf(chp, "off, ");
                }else{
                    chprintf(chp, "on, ");
                }
                if(palReadPad(GPIOE, fault[i])){
                    chprintf(chp, "nominal\r\n");
                }else{
                    chprintf(chp, "fault\r\n");
                }
            }
        }
    }
}

static void cmd_KS8999(BaseSequentialStream *chp, int argc, char *argv[]){
    if(argc < 1 || argc > 3){
        chprintf(chp, "Usage: ksz [rst] [pwr] [on|off]\r\n");
        return;
    }

    int rst = FALSE;
    int pwr = FALSE;
    typedef enum{info, on, off} action;
    action act = info;

    //parse arguments
    int i;
    for(i = 0; i < argc; ++i){
        if(!strcmp(argv[i], "rst")){
            rst = TRUE;
        }else if(!strcmp(argv[i], "pwr")){
            pwr = TRUE;
        }else if(i == argc - 1){
            if(!strcmp(argv[i], "on")){
                act = on;
            }else if(!strcmp(argv[i], "off")){
                act = off;
            }else{
                chprintf(chp, "Unrecognized action: %s. Valid acts are [on|off]\r\n", argv[i]);
                return;
            }
        } else {
            chprintf(chp, "Unrecognized pin: %s. Valid pins are rst or pwr\r\n", argv[i]);
            return;
        }
    }
    // ksz on|off == ksz rst pwr on|off
    if(rst == FALSE && pwr == FALSE){
        rst = TRUE;
        pwr = TRUE;
    }

    //do actions
    if(pwr){
        switch(act){
        case on:
            KS8999_power(TRUE);
            break;
        case off:
            KS8999_power(FALSE);
            break;
        case info:
        default:
            chprintf(chp, "pwr: ");
            if(palReadPad(GPIOD, GPIO_D14_KSZ_EN)){
                chprintf(chp, "on\r\n");
            }else{
                chprintf(chp, "off\r\n");
            }
        }
    }

    if(rst){
        switch(act){
        case on:
            palSetPad(GPIOD, GPIO_D4_ETH_N_RST);
            break;
        case off:
            palClearPad(GPIOD, GPIO_D4_ETH_N_RST);
            break;
        case info:
        default:
            chprintf(chp, "rst: ");
            if(palReadPad(GPIOD, GPIO_D4_ETH_N_RST)){
                chprintf(chp, "off\r\n");
            }else{
                chprintf(chp, "on\r\n");
            }
        }
    }

}


#define SHELL_WA_SIZE THD_WA_SIZE(2048)
static int shell_init = FALSE;
static Thread *shelltp = NULL;
void rnh_shell_start(void){
    // XXX: Is there a better way to check if the hardware is initialized?
    if(!shell_init){
        sdStart(&SD1, NULL);
        shellInit();
        shell_init = TRUE;
    }

    static const ShellCommand commands[] = {
            {"threads", cmd_threads},
            {"pwr", cmd_power},
            {"ksz", cmd_KS8999},
            {NULL, NULL}
    };

    static const ShellConfig shell_cfg1 = {
            (BaseSequentialStream *)&SD1,
            commands
    };

    if (!shelltp)
        shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminated(shelltp)) {
        chThdRelease(shelltp);    /* Recovers memory of the previous shell.  */
        shelltp = NULL;           /* Triggers spawning of a new shell.       */
    }
}
