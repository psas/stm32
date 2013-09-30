/*! \file sdcdetail.c
 *  SD Card
 */

/*!
 * \defgroup sdcdetail SDC Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"
#include "usbdetail.h"
#include "sdcdetail.h"



/**
 ** Definitions
 **************/

#define   DEBUG_SDC                 FALSE
#define   SDC_CONNECT_MAX_TRIES     2

// The following two definitions are in units of ms.
#define   SDC_POLLING_INTERVAL      10
#define   SDC_POLLING_DELAY         10

// This is the number of times in a row that we need to see an inserted SD Card
// before we will trigger the sdc_inserted_event. The product of SDC_DEBOUNCE
// and SDC_POLLING_INTERVAL gives the total minimum delay for that event firing
// after an SD card has been inserted.
#define   SDC_DEBOUNCE              10



/**
 ** Public Global Variables
 **************************/

// Used to signal that the SD card is mounted and ready to be written to.
bool            fs_ready            = FALSE;

EventSource     sdc_inserted_event;
EventSource     sdc_removed_event;
FATFS           SDC_FS;



/**
 ** Private Global Variables & Functions
 ***************************************/

static  VirtualTimer    sdc_monitor_tmr;
static  void            sdc_monitor(void *p);



/**
 ** Public Function Definitions
 ******************************/

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
void sdc_init(void *p) {
  chEvtInit(&sdc_inserted_event);
  chEvtInit(&sdc_removed_event);

  chSysLock();
  chVTSetI(&sdc_monitor_tmr, MS2ST(SDC_POLLING_DELAY), sdc_monitor, p);
  chSysUnlock();
}


/*!
 * \brief Card insertion event.
 */
void InsertHandler(eventid_t _) {
  FRESULT mount_status;

  /*
   * First initialize the SD Card.
   * We try this a few times because otherwise connecting to the card fails
   * every other time the card is inserted, but succeeds otherwise.
   * TODO: figure out what's up with that.
   */
  char i = 0;
  while (sdcConnect(&SDCD1)) {
    if (++i >= SDC_CONNECT_MAX_TRIES) return;
  }

  /*
   * Now mount the FAT FS that's on the card.
   */
  mount_status = f_mount(0, &SDC_FS);
  if (mount_status != FR_OK) {
    sdcDisconnect(&SDCD1);
    return;
  }

  fs_ready = TRUE;
}


/*!
 *  \brief Card removal event.
 */
void RemoveHandler(eventid_t _) {
  sdcDisconnect(&SDCD1);
  fs_ready = FALSE;
}



/**
 ** Private Function Definitions
 *******************************/

/*!
 * @brief   Card insertion status monitor timer callback function.
 *
 * @param[in] p         pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void sdc_monitor(void *p) {
  static unsigned char debounce_count = SDC_DEBOUNCE;
  BaseBlockDevice *bbdp = p;

  // TODO: why do we have to do this?
  chSysLockFromIsr();

  /*
   * This is a check to see if we think the card is inserted.
   * If the debounce_count is greater than zero, that means we think it's not
   * inserted; if it's zero, then we think that it is.
   */
  if (debounce_count > 0) {
    if (blkIsInserted(bbdp)) {
      // if the card still seems to be inserted, decrement the debounce count
      if (--debounce_count == 0) {
        // if the debounce count is zero, we're confident that the card is
        // actually inserted for reals
        chEvtBroadcastI(&sdc_inserted_event);
      }
    } else {
      // if the card isn't inserted, reset the debounce count
      debounce_count = SDC_POLLING_INTERVAL;
    }
  } else {
    // in this branch, we believe that the card has been inserted
    if (!blkIsInserted(bbdp)) {
      // if it appears not to be, reset the debounce count and then trigger the
      // sdc_removed_event
      debounce_count = SDC_POLLING_INTERVAL;
      chEvtBroadcastI(&sdc_removed_event);
    }
  }

  chVTSetI(&sdc_monitor_tmr, MS2ST(SDC_POLLING_INTERVAL), sdc_monitor, bbdp);
  chSysUnlockFromIsr();
}


//! @}
