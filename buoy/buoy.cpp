/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include <stdint.h>
# include <stdlib.h>

# include "buoy.h"

# include "wirish.h"
# include "iwdg.h"

# if HASRF
  # include "rf.h"
# endif

# if HASGPS
  # include "gps.h"
# endif

# include "ads1282.h"
# include "store.h"

namespace Buoy {
  void BuoyMaster::main () {
    setup ();

    while (true) loop ();

  }

  void BuoyMaster::loop () {
    static uint32_t lasts = 0;

    critical_loop ();

# if HASRF
    rf->loop ();
# endif

    ad->loop ();

    if (millis () - lasts >= 1000) {
      SerialUSB.print ("V");
      SerialUSB.println (ad->values[ad->position-1]);
      SerialUSB.println (ad->position);
      //ad->print_status ();
      //gps->print_status ();
      lasts = millis ();
    }
  }

  void BuoyMaster::critical_loop () {
    /* Critical functions */
# if HASGPS
    gps->loop ();
# endif

    store->loop ();

    iwdg_feed (); // reset watchdog
  }

  void BuoyMaster::setup () {
    /* Configure LED */
    //SerialUSB.begin ();

    pinMode (13, OUTPUT);
    pinMode (3, OUTPUT);
    digitalWrite (3, LOW);
    /* Count down.. */

    for (int i = 0; i < 3; i++) {
      //SerialUSB.print ("S ");
      SerialUSB.println (i);
      delay(1000);
      //togglePin (3);
    }

    SerialUSB.println ("Gautebøye [" STRINGIFY(BUOY) "] " GIT_DESC);

    /* Set up devices */
# if HASRF
    rf    = new RF ();
# endif
# if HASGPS
    gps   = new GPS ();
# endif
    store = new Store ();
    ad    = new ADS1282 ();

# if HASRF
    rf->setup     (this);
# endif
# if HASGPS
    gps->setup    (this);
# endif
    ad->setup     (this);
    store->setup  (this);

    /* Set up independent watchdog */
    iwdg_init (IWDG_PRE_256, WATCHDOG_RELOAD);

# if DEBUG_INFO
    SerialUSB.println ("[Buoy] Initiating continuous transfer and write.");
# endif

    /* Start reading data continuously and writing to SD card */
    store->start_continuous_write ();
    ad->start_continuous_read ();
  }

  inline bool BuoyMaster::hasusb () {
    return SerialUSB.isConnected ();
  }

  int itoa (uint32_t n, uint8_t base, char *buf) // {{{
  {
    unsigned long i = 0;

    if (n == 0) {
      buf[i++] = '0';
    }

    while (n > 0) {
      buf[i] = (n % base);
      buf[i] += (buf[i] < 10 ? '0' : 'A');
      n /= base;
      i++;
    }

    /* swap */
    i--;
    char c;
    while (n <= (i/2)) {
      c         = buf[i-n];
      buf[i-n]  = buf[n];
      buf[n]    = c;

      n++;
    }

    buf[++i] = 0;
    return i;
  } // }}}

  /* Checksum {{{ */
  byte gen_checksum (const char *buf)
  {
  /* Generate checksum for NULL or * terminated string */

    byte csum = 0;
    buf++; // skip $

    while (*buf != '*' && *buf != 0) {
      csum = csum ^ ((byte)*buf);
      buf++;
    }

    return csum;
  }

  bool test_checksum (const char *buf)
  {
    /* Input: String including $ and * with HEX decimal checksum
     *        to test. NULL terminated.
     */
    uint8_t  n = 0;
# define MAX_CHARS 80
    uint32_t tsum = 0;
    buf++; // skip $
    while (*buf != '*' && *buf != 0 && n < MAX_CHARS) {
      tsum = tsum ^ (uint8_t)*buf;
      buf++;
      n++;
    }
    buf++;

    uint16_t csum = 0;
    csum = strtoul (buf, NULL, 16); // buf now points to first digit of CS

    return tsum == csum;
  } // }}}
}

/* vim: set filetype=arduino :  */

