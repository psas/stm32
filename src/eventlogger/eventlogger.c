#include <time.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "chrtclib.h"

#include "psas_rtc.h"
#include "usbdetail.h"

#include "eventlogger.h"



/**
 ** Definitions
 **************/

#define EVENTBUFF_LENGTH 64
#define EVENTLOG_DEBUG true



/**
 ** Private Function Declarations
 ********************************/

/*
 * This is the logger thread that writes the posted events to the SD card.
 */
static msg_t log_event(void *_);



/**
 ** Private Global Variables
 ***************************/

/*
 * This here mailbox is the hidden point of contact between the public
 * post_event() function and the private log_event().
 */
static msg_t event_buffer[EVENTBUFF_LENGTH];
static MAILBOX_DECL(event_mail, event_buffer, EVENTBUFF_LENGTH);

static WORKING_AREA(wa_thread_log_event, 512);



/**
 ** Public Function Definitions
 ******************************/

/*
 * post_event
 *
 * This is how you send events to the logger.
 * For now, the event_t e is just an opaque int.
 * TODO: figure out data format.
 * The returned boolean indicates whether the event was succesfully posted
 * (true) or failed because the buffer of events to be logged was full (false).
 */
bool post_event(event_t e) {
  msg_t status;
  RTCTime posted_at_rtc;
  struct tm posted_at_tm;
  time_t posted_at_unix;

  psas_rtc_lld_get_time(&RTCD1, &posted_at_rtc);
  psas_stm32_rtc_bcd2tm(&posted_at_tm, &posted_at_rtc);
  posted_at_unix = mktime(&posted_at_tm);

  status = chMBPost(&event_mail, e, TIME_IMMEDIATE);
  return status == RDY_OK;
}


/*
 * eventlogger_init
 *
 * This must be called before post_event can be used.
 * It sets up the logger thread.
 * It assumes that Chibi's RTOS kernel has already been activated by calling
 * chSysInit().
 */
void eventlogger_init(void) {
  chThdCreateStatic( wa_thread_log_event
                   , sizeof(wa_thread_log_event)
                   , NORMALPRIO
                   , log_event
                   , NULL
                   );
}



/**
 ** Private Function Definitions
 *******************************/

/*
 * log_event
 *
 * Just pulls messages out of the mailbox for now.
 * TODO: actually log things to the SD card.
 */
static msg_t log_event(void *_) {
  event_t posted;
  RTCTime received_at_rtc;
  struct tm received_at_tm;
  time_t received_at_unix;

  chRegSetThreadName("eventlogger");

  while (true) {
    // TODO: figure out if chMBFetch does the sensible thing and puts the thread
    // to sleep until something gets posted to the mailbox.
    chMBFetch(&event_mail, (msg_t *) &posted, TIME_INFINITE);

    psas_rtc_lld_get_time(&RTCD1, &received_at_rtc);
    psas_stm32_rtc_bcd2tm(&received_at_tm, &received_at_rtc);
    received_at_unix = mktime(&received_at_tm);

#if EVENTLOG_DEBUG
    chprintf( (BaseSequentialStream *) &SDU_PSAS
            , "\"logging\" event from %D.%06D\r\n"
            , received_at_unix
            , received_at_rtc.tv_msec
            );
#endif
  }

  return -1;
}
