
#include "ch.h"
#include "chprintf.h"
#include "utils_general.h"

/*! \brief Show memory usage
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[] UNUSED) {
	size_t n, size;

	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments	: %u\r\n", n);
	chprintf(chp, "heap free total	: %u bytes\r\n", size);
}


/*! \brief Show running threads
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[] UNUSED) {
	static const char *states[] = {THD_STATE_NAMES};
	Thread *tp;

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

#if 0
static time_t unix_time;

static uint8_t fbuff[1024];

#define MAX_FILLER 11

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
	FRESULT err;
	unsigned long clusters;
	unsigned long long total = 0;
	FATFS *fsp;

	(void)argv;
	if (argc > 0) {
		SHELLDBG("Usage: tree\r\n");
		return;
	}
	if (!fs_ready) {
		SHELLDBG("File System not mounted\r\n");
		return;
	}
	err = f_getfree("/", &clusters, &fsp);
	if (err != FR_OK) {
		err = f_getfree("/", &clusters, &fsp);
		if (err != FR_OK) {
			SHELLDBG("FS: f_getfree() failed. FRESULT: %d\r\n", err);
			return;
		}
	}
	SHELLDBG("FS: %lu free clusters, %lu sectors per cluster, %lu bytes free.\r\n",
		clusters, (uint32_t)SDC_FS.csize,
		total);
	fbuff[0] = 0;
	sdc_scan_files(chp, (char *)fbuff);
}

void cmd_sdchalt(BaseSequentialStream *chp, int argc, char *argv[]){
	(void)argv;
	(void)argc;
	(void)chp;

	sdc_haltnow() ;

	SHELLDBG("SDC card system halted. Remove card and restart system before inserting again.\r\n");
}


void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
	(void)argv;
	(void)chp;
  struct tm timp;
  RTCTime psas_time;

  if (argc == 0) {
	goto ERROR;
  }

  if ((argc == 1) && (strcmp(argv[0], "get") == 0)){
	  psas_rtc_lld_get_time(&RTCD1, &psas_time);
	  psas_stm32_rtc_bcd2tm(&timp, &psas_time);

	  unix_time = mktime(&timp);

	  if (unix_time == -1){
		  SHELLDBG("incorrect time in RTC cell\r\n");
	  }
	  else{
		  SHELLDBG("%Ds %Dus %s",unix_time, psas_time.tv_msec, " - unix time\r\n");
		  SHELLDBG("%lu\r\n", psas_rtc_s.fc_boot_time_mark );
		  rtcGetTimeTm(&RTCD1, &timp);
		  SHELLDBG("%s%s",asctime(&timp)," - formatted time string\r\n");
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
	SHELLDBG("Usage: date get\r\n");
	SHELLDBG("     date set N\r\n");
	SHELLDBG("where N is time in seconds sins Unix epoch\r\n");
	SHELLDBG("you can get current N value from unix console by the command\r\n");
	SHELLDBG("%s", "date +\%s\r\n");
	return;
}
#endif
