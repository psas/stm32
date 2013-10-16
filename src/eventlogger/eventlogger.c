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

typedef struct {
  uint32_t seconds;
  uint32_t packed_us_event;
} Event;



/**
 ** Private Function Declarations
 ********************************/

/*
 * This is the logger thread that writes the posted events to the SD card.
 */
static msg_t log_event(void* _);

/*
 * This creates an Event struct, gets the current time from the RTC, and packs
 * the fields together.
 */
Event* make_event(event_t et);



/**
 ** Private Macro Definitions
 ****************************/

#define pack_event_and_us(et, us) ((uint32_t) ((us & ((1 << 20) - 1)) << 8 | (et & 0xff)))
#define unpack_event(et_us)       ((uint8_t)  et_us & 0xff)
#define unpack_us(et_us)          ((uint32_t) et_us >> 8)



/**
 ** Private Global Variables
 ***************************/

/*
 * This here mailbox is the hidden point of contact between the public
 * post_event() function and the private log_event().
 */
static msg_t event_buffer[EVENTBUFF_LENGTH];
static MAILBOX_DECL(event_mail, event_buffer, EVENTBUFF_LENGTH);

static MemoryPool event_pool;
/*
 * This is the static storage that backs the events' memory pool.
 * The size of this array defines the maximum number of Event objects that can
 * be allocated at any one time.
 * Since each message posted to the above mailbox will refer to one Event, this
 * array should have the same size as the mailbox.
 */
static Event event_data[EVENTBUFF_LENGTH] __attribute__((aligned(sizeof(stkalign_t))));

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
bool post_event(event_t et) {
  msg_t status;

  Event* e = make_event(et);
  if (e == NULL) return false;

  status = chMBPost(&event_mail, (msg_t) e, TIME_IMMEDIATE);

  return status == RDY_OK;
}


/*
 * eventlogger_init
 *
 * This must be called before post_event can be used.
 * It sets up the logger thread and Event memory pool.
 * It assumes that ChibiOs's RTOS kernel has already been activated by calling
 * chSysInit().
 */
void eventlogger_init(void) {
  chPoolInit(&event_pool, sizeof(Event), NULL);
  chPoolLoadArray(&event_pool, event_data, EVENTBUFF_LENGTH);

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
  Event* posted;
  time_t curr_s;
  uint32_t e_us, curr_us;
  int32_t us_diff;
  uint8_t et;

  chRegSetThreadName("eventlogger");

  while (true) {
    // TODO: figure out if chMBFetch does the sensible thing and puts the thread
    // to sleep until something gets posted to the mailbox.
    chMBFetch(&event_mail, (msg_t *) &posted, TIME_INFINITE);

    e_us = unpack_us(posted->packed_us_event);
    et = unpack_event(posted->packed_us_event);

    psas_rtc_lld_get_s_and_us(&RTCD1, &curr_s, &curr_us);
    us_diff = curr_us - e_us;
    us_diff = (us_diff < 0) ? 1000000 - us_diff : us_diff;

#if EVENTLOG_DEBUG
    chprintf( (BaseSequentialStream *) &SDU_PSAS
            , "\"logging\" event %03d from %D.%06D delay %D.%06D s\r\n"
            , et
            , posted->seconds
            , e_us
            , curr_s - posted->seconds
            , us_diff
            );
#endif

    chPoolFree(&event_pool, posted);
  }

  return -1;
}

Event* make_event(event_t et) {
  Event* e = (Event *) chPoolAlloc(&event_pool);

  // bail if the pool is empty
  if (e == NULL) return NULL;

  time_t s;
  uint32_t us;
  psas_rtc_lld_get_s_and_us(&RTCD1, &s, &us);

  e->seconds = s;
  e->packed_us_event = pack_event_and_us(et, us);

  return e;
}
