/*
 */
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "KS8999.h"
#include "BQ24725.h"
#include "BQ3060.h"
#include "RNHPort.h"
#include "rnh_shell.h"

#define UNUSED __attribute__((unused))

void cmd_bq_did(BaseSequentialStream *chp, int argc, char *argv[] UNUSED){
    if (argc > 0) {
        chprintf(chp, "Usage: bq_did. Should return 0x8\n");
        return;
    }

    uint16_t data;
    int err = BQ24725_GetDeviceID(&data);
    if(err){
//#define I2CD_BUS_ERROR              0x01   /**< @brief Bus Error.           */
//#define I2CD_ARBITRATION_LOST       0x02   /**< @brief Arbitration Lost.    */
//#define I2CD_ACK_FAILURE            0x04   /**< @brief Acknowledge Failure. */
//#define I2CD_OVERRUN                0x08   /**< @brief Overrun/Underrun.    */
//#define I2CD_PEC_ERROR              0x10   /**< @brief PEC Error in
//                                                reception.                  */
//#define I2CD_TIMEOUT                0x20   /**< @brief Hardware timeout.    */
//#define I2CD_SMB_ALERT              0x40   /**< @brief SMBus Alert.         */
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "GOT SOMETHING COOL: 0x%x\n", data);
    }
}

void cmd_bq_mid(BaseSequentialStream *chp, int argc, char *argv[] UNUSED){
    if (argc > 0) {
        chprintf(chp, "Usage: bq_mid. Should return 0x40\n");
        return;
    }
    uint16_t data;
    int err = BQ24725_GetManufactureID(&data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "GOT SOMETHING COOL: 0x%x\n", data);
    }
}

void cmd_3060_did(BaseSequentialStream *chp, int argc, char *argv[] UNUSED){
    if (argc > 0) {
        chprintf(chp, "Usage: bq_mid. Should return 0x40\n");
        return;
    }
    uint16_t data;
    int err = BQ3060_Get(0x1c, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "GOT SOMETHING COOL: 0x%x\n", data);
    }
}

void cmd_3060_safety(BaseSequentialStream *chp, int argc, char *argv[] UNUSED){
    if (argc > 0) {
        chprintf(chp, "Usage: bq_mid. Should return 0x40\n");
        return;
    }
    uint16_t data;
    int err = BQ3060_Get(0x50, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "SA:0x%x ", data);
    }
    err = BQ3060_Get(0x51, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "SS:0x%x ", data);
    }
    err = BQ3060_Get(0x52, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "PFA:0x%x ", data);
    }
    err = BQ3060_Get(0x53, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "PFS:0x%x\n", data);
    }
}

void cmd_3060_temp(BaseSequentialStream *chp, int argc, char *argv[] UNUSED){
    if (argc > 0) {
        chprintf(chp, "Usage: bq_mid. Should return 0x40\n");
        return;
    }
    uint16_t data;
    int err = BQ3060_Get(0x08, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "temp:%dk/10 ", data);
    }
    err = BQ3060_Get(0x5e, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "pack1:%dk/10 ", data);
    }
    err = BQ3060_Get(0x5f, &data);
    if(err){
        chprintf(chp, "GOT SOMETHING BAD: 0x%x\n", err);
    }
    else{
        chprintf(chp, "pack2:%dk/10\n", data);
    }
}

void cmd_acok(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: acok\n");
        return;
    }
    int state = palReadPad(GPIOD, GPIO_D0_BQ24_ACOK);
    switch(state){
    case PAL_HIGH:
        chprintf(chp, "high\n");
        break;
    case PAL_LOW:
        chprintf(chp, "low\n");
        break;
    default:
        chprintf(chp, "unknown\n");
    }
}

void cmd_bq_charge(BaseSequentialStream *chp, int argc UNUSED, char *argv[] UNUSED){
    BQ24725_charge_options BQ24725_rocket_init = {
                .ACOK_deglitch_time = t150ms,
                .WATCHDOG_timer = disabled,
                .BAT_depletion_threshold = FT70_97pct,
                .EMI_sw_freq_adj = dec18pct,
                .EMI_sw_freq_adj_en = sw_freq_adj_disable,
                .IFAULT_HI_threshold = l700mV,
                .LEARN_en = LEARN_disable,
                .IOUT = adapter_current,
                .ACOC_threshold = l1_66X,
                .charge_inhibit = charge_enable
            };
            BQ24725_SetChargeCurrent(0x400);
            chprintf(chp, "Charge current set\n");
            BQ24725_SetChargeVoltage(0x41A0);
            chprintf(chp, "Charge voltage set\n");
            BQ24725_SetInputCurrent(0x0A00);
            chprintf(chp, "Input Current set\n");
            BQ24725_SetChargeOption(&BQ24725_rocket_init);
            chprintf(chp, "Charge options set\n");
}

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

    typedef enum {info, on, off} action;
    action act = info;
    RNHPort port = 0;
    //parse arguments
    for(int i = 0; i < argc; ++i){
        if(!strcmp(argv[i], "1")){
            port |= RNH_PORT_1;
        }else if(!strcmp(argv[i], "2")){
            port |= RNH_PORT_2;
        }else if(!strcmp(argv[i], "3")){
            port |= RNH_PORT_3;
        }else if(!strcmp(argv[i], "4")){
            port |= RNH_PORT_4;
        }else if(!strcmp(argv[i], "6")){
            port |= RNH_PORT_6;
        }else if(!strcmp(argv[i], "7")){
            port |= RNH_PORT_7;
        }else if(!strcmp(argv[i], "all")){
            port |= RNH_PORT_ALL;
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
    RNHPort status = 0;
    RNHPort fault = 0;
    switch(act){
    case on:
        rnhPortOn(port);
        break;
    case off:
        rnhPortOff(off);
        break;
    case info:
    default:
        status = rnhPortStatus();
        fault = rnhPortFault();
        for(int i = 0; i < 8; ++i){
            chprintf(chp, "%d: ", i + 1);
            if(status & 1<<i){
                chprintf(chp, "off, ");
            }else{
                chprintf(chp, "on, ");
            }
            if(fault & 1<<i){
                chprintf(chp, "nominal\r\n");
            }else{
                chprintf(chp, "fault\r\n");
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
            {"bq_did", cmd_bq_did},
            {"bq_mid", cmd_bq_mid},
            {"3060_did", cmd_3060_did},
            {"3060_s", cmd_3060_safety},
            {"3060_temp", cmd_3060_temp},
            {"acok", cmd_acok},
            {"charge", cmd_bq_charge},
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
