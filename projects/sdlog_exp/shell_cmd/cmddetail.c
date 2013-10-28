/*! \file cmddetail.c
 *
 */

/*!
 * \defgroup cmddetail Command Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "mac.h"
#include "ff.h"
#include "sdcdetail.h"

#include "chrtclib.h"

#include "cmddetail.h"

#include "psas_sdclog.h"
#include "psas_rtc.h"

#define 		DEBUG_PHY 			0

static time_t      unix_time;

static uint8_t     fbuff[1024];

#define SDC_TESTFILE          "cmd_test.txt"
#define SDC_NEWFILE           "/NEWFILE.txt"
#define SDC_BIGFILE           "/BIGFILE.txt"
#define SDC_NEWPOSITION       10
void cmd_sdct(BaseSequentialStream *chp, int argc, char *argv[]) {
    FRESULT      rc;
    FIL          Fil;
    uint8_t      buff[1];
    unsigned     i  = 0;
    unsigned     br = 0;
    unsigned     bw = 0;

    chprintf(chp, "%s: sizeof unsigned long: %d\r\n", __func__, sizeof(unsigned long));

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: sdct\r\n");
        return;
    }
    chprintf(chp, "\r\nOpen an existing file: %s.\r\n", SDC_TESTFILE);
    rc = f_open(&Fil, SDC_TESTFILE, FA_READ);
    if (rc) {
        chprintf(chp, "\r\nOpen an existing file FAILED.\r\n");
        chprintf(chp, "\tError %d\r\n", rc);
        return;
    }

    chprintf(chp, "\r\nType the file content.\r\n");
    for (;;) {
        rc = f_read(&Fil, buff, sizeof buff, &br);      /* Read a chunk of file */
        if (rc || !br) break;                           /* Error or end of file */
        for (i = 0; i < br; i++)                        /* Type the data */
            chprintf(chp, "%c", buff[i]);
    }
    if (rc) {
        chprintf(chp, "\r\nContents of an existing file FAILED.\r\n");
        chprintf(chp, "\tError %d\r\n", rc);
        return;
    }

    chprintf(chp, "\r\nClose an existing file: %s.\r\n", SDC_TESTFILE);
    rc = f_close (&Fil);
    if (rc) {
        chprintf(chp, "\r\nClose an existing file FAILED. %s.\r\n", SDC_TESTFILE);
        chprintf(chp, "\tError %d\r\n", rc);
    }

    /* Move to end of the file to append data */
    //   res = f_lseek(fp, f_size(fp));
    // create a file write to it, close it open it and look at the results.
    rc = f_open(&Fil, SDC_BIGFILE, FA_WRITE);
    if (rc) {
        chprintf(chp, "\r\nOpen an existing file FAILED. %s.\r\n", SDC_BIGFILE);
        chprintf(chp, "\tError %d\r\n", rc);
    }

    chprintf(chp, "\nMove to new filesize.\r\n");

    rc = f_lseek(&Fil, SDC_NEWPOSITION);
    if (rc)  {
           chprintf(chp, "\r\nf_lseek to %lu fail... %s.\r\n", SDC_NEWFILE, SDC_NEWPOSITION);
           chprintf(chp, "\tError %d\r\n", rc);
    }


    chprintf(chp, "\nWrite a text data. (Hello world!)\n");
    rc = f_write(&Fil, "Hello world!\r\n", 14, &bw);
    if (rc)  {
        chprintf(chp, "\r\nWrite an existing file FAILED. %s.\r\n", SDC_BIGFILE);
        chprintf(chp, "\tError %d\r\n", rc);
    }
    chprintf(chp, "%u bytes written.\n", bw);
    chprintf(chp, "\r\nClose an existing file: %s.\r\n", SDC_BIGFILE);
    rc = f_close (&Fil);
    if (rc) {
        chprintf(chp, "\r\nClose an existing file FAILED. %s.\r\n", SDC_BIGFILE);
        chprintf(chp, "\tError %d\r\n", rc);
    }


    return;
}

#define MAX_FILLER 11
static char *long_to_string_with_divisor(BaseSequentialStream *chp,
                                         char *p,
                                         unsigned long long num,
                                         unsigned radix,
                                         long divisor) {
  unsigned long long i;
  char *q;
  unsigned long long l, ll;
  char tmpbuf[MAX_FILLER + 1];

  tmpbuf[MAX_FILLER] = '\0';
  p = tmpbuf;
  q = tmpbuf;

  l = num;
  if (divisor == 0) {
    ll = num;
  } else {
    ll = divisor;
  }

  q = p + MAX_FILLER;
  do {
    i =  (unsigned long long)(l % radix);
    i += '0';
    if (i > '9')
      i += 'A' - '0' - 10;
    *--q = i;
    l /= radix;
  } while ((ll /= radix) != 0);

  i = (unsigned long long) (p + MAX_FILLER - q);
  do {
    *p++ = *q++;
  } while (--i);
  chprintf(chp, "%s: %s\r\n", __func__, tmpbuf);
  return p;
}

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
    FRESULT err;
    unsigned long clusters;
    unsigned long long total;
    FATFS *fsp;
    int howbig;
    char* p;
   // char buffern[20];

    (void)argv;
    if (argc > 0) {
            chprintf(chp, "Usage: tree\r\n");
            return;
    }
    if (!fs_ready) {
            chprintf(chp, "File System not mounted\r\n");
            return;
    }
    err = f_getfree("/", &clusters, &fsp);
    if (err != FR_OK) {
            err = f_getfree("/", &clusters, &fsp);
            if (err != FR_OK) {
                    chprintf(chp, "FS: f_getfree() failed. FRESULT: %d\r\n", err);
                    return;
            }
    }
    chprintf(chp, "ULONG_MAX: %lu\n", ULONG_MAX);
    total =  1936690ULL * 8ULL * 512ULL;
    //total = clusters * (uint32_t)SDC_FS.csize * (uint32_t)MMCSD_BLOCK_SIZE;
    p = long_to_string_with_divisor(chp, p, total, 10, 0);
    howbig = sizeof(unsigned long);
    chprintf(chp, "howbig: %i\r\n", howbig);
    chprintf(chp,
            "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free.\r\n",
            clusters, (uint32_t)SDC_FS.csize,
            total);
    fbuff[0] = 0;
    sdc_scan_files(chp, (char *)fbuff);
}


void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)argv;
  struct   tm timp;
  RTCTime   psas_time;

  if (argc == 0) {
    goto ERROR;
  }


  if ((argc == 1) && (strcmp(argv[0], "get") == 0)){
	  psas_rtc_lld_get_time(&RTCD1, &psas_time);
      psas_stm32_rtc_bcd2tm(&timp, &psas_time);

      unix_time = mktime(&timp);

      if (unix_time == -1){
          chprintf(chp, "incorrect time in RTC cell\r\n");
      }
      else{
          chprintf(chp, "%Ds %Dus %s",unix_time, psas_time.tv_msec, " - unix time\r\n");
          rtcGetTimeTm(&RTCD1, &timp);
          chprintf(chp, "%s%s",asctime(&timp)," - formatted time string\r\n");
      }
      // }
      return;
  }

  if ((argc == 2) && (strcmp(argv[0], "set") == 0)){
    unix_time = atol(argv[1]);
    if (unix_time > 0){
      rtcSetTimeUnixSec(&RTCD1, unix_time);
      return;
    }
    else{
      goto ERROR;
    }
  }
  else{
    goto ERROR;
  }

ERROR:
  chprintf(chp, "Usage: date get\r\n");
  chprintf(chp, "       date set N\r\n");
  chprintf(chp, "where N is time in seconds sins Unix epoch\r\n");
  chprintf(chp, "you can get current N value from unix console by the command\r\n");
  chprintf(chp, "%s", "date +\%s\r\n");
  return;
}

/*! \brief Show memory usage
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

/*! \brief Show running threads
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
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


/*! \brief   Read a PHY register.
 *
 */
static uint32_t mii_read(MACDriver *macp, uint32_t reg) {

 ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR | ETH_MACMIIAR_MB;
 while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
   ;
 return ETH->MACMIIDR;
}

#if DEBUG_PHY
/*! \brief   Writes a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 * @param[in] value     new register value
 */
static void mii_write(MACDriver *macp, uint32_t reg, uint32_t value) {

  ETH->MACMIIDR = value;
  ETH->MACMIIAR = macp->phyaddr | (reg << 6) | MACMIIDR_CR |
                  ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;
  while ((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
	  ;
}
#endif

void cmd_phy(BaseSequentialStream *chp, int argc, char *argv[]) {
	uint32_t phy_val     = 0;
	uint32_t reg_to_ping = 0;

	//uint32_t bmcr_val = 0;

	if (argc != 1) {
		chprintf(chp, "Usage: phy reg(decimal)\r\n");
		return;
	}

//	bmcr_val = mii_read(&ETHD1, MII_BMCR);
//
//	mii_write(&ETHD1, MII_BMCR, (bmcr_val & ~(1<<12)) );
//
//	bmcr_val = mii_read(&ETHD1, MII_BMCR);
//
//	mii_write(&ETHD1, 0x1f,( bmcr_val | 1<<13));

	reg_to_ping = atoi(argv[0]);
	phy_val = mii_read(&ETHD1, reg_to_ping);
	chprintf(chp, "phy reg 0x%x value:\t0x%x\n\r", reg_to_ping, phy_val);

}
//! @}
