/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

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

    uint32_t iter = 0;
# define LOOP_DELAY 10

    while (true) {
      ad->loop ();
      rf->loop ();
      gps->loop ();
      store->loop ();

      delay (LOOP_DELAY);

      /* Give GPS a chance to get seconds or sync */
      if (!ad->continuous_read && ((iter * LOOP_DELAY) > 1000)) {
        ad->start_continuous_read ();
      }

      iter++;
    }
  }

  void BuoyMaster::setup () {
    /* Configure LED */
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite (BOARD_LED_PIN, LOW);

    /* Set up devices */
    rf = new RF ();
    rf->setup (this);

    gps = new GPS ();
    gps->setup (this);

    ad = new ADS1282 ();
    ad->setup (this);

    store = new Store ();
    store->setup (this);

# if DIRECT_SERIAL
    SerialUSB.println ("[Buoy] Initiating continuous transfer and write.");
# endif

    rf->send_debug ("[Buoy] All subsystems initiated.");
    //store->log ("[Buoy] All subsystems initiated.");

    rf->send_debug ("[Buoy] Initiating continuos transfer and write.");
    //store->log ("[Buoy] Initiating continuos transfer and write.");

    rf->start_continuous_transfer ();
    store->start_continuous_write ();
  }

  void BuoyMaster::send_greeting () {
    rf->send_debug ("[Buoy] Initializing Gautebuoy [ " BUOY_ID_S ": " BUOY_NAME " ] ( version " VERSION " )");
  }
}

/* vim: set filetype=arduino :  */

