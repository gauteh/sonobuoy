/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include <limits.h> // CHAR_BIT
# include <stdint.h>

# include "wirish.h"
# include "buoy.h"
# include "ads1282.h"
# include "rf.h"
# include "gps.h"
# include "store.h"

namespace Buoy {
  BuoyMaster::BuoyMaster () {

  }

  void BuoyMaster::main () {
    delay (3000); // time to connect with serial before starting..
    setup ();

# define LOOP_DELAY 10

    while (true) {
      gps->loop ();
      ad->loop ();

      rf->loop ();
      store->loop ();

      delay (LOOP_DELAY);
    }
  }

  void BuoyMaster::setup () {
    /* Configure LED */
    pinMode (BOARD_LED_PIN, OUTPUT);
    digitalWrite (BOARD_LED_PIN, LOW);

    /* Set up devices */
    rf = new RF ();
    gps = new GPS ();
    ad = new ADS1282 ();
    store = new Store ();

    rf->setup (this);
    gps->setup (this);
    ad->setup (this);
    store->setup (this);

    SerialUSB.println ("[Buoy] Initiating continuous transfer and write.");

    ad->start_continuous_read ();
    store->start_continuous_write ();
  }


  void BuoyMaster::send_greeting () {
    rf->send_debug ("[Buoy] Initializing Gautebuoy [ " BUOY_ID_S ": " BUOY_NAME " ] ( version " VERSION " )");
  }

  int itoa (uint32_t n, uint8_t base, char *buf)
  {
    unsigned long i = 0;

    if (n == 0) {
      buf[i++] = '0';
    }

    while (n > 0) {
      buf[i] = (n % base);
      buf[i]  += (buf[i] < 10 ? '0' : 'A');
      n /= base;
      i++;
    }

    /* swap */
    i--;
    char c;
    while ( n < (i/2) ) {
      c = buf[i-n];
      buf[i-n] = buf[n];
      buf[n] = c;

      n++;
    }

    buf[++i] = 0;
    return i;
  }
}

/* vim: set filetype=arduino :  */

