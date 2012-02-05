/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include "wirish.h"
# include "buoy.h"
#
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


    while (true) {
      ad->loop ();
      rf->loop ();
      gps->loop ();

      delay (10);

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
    store->setup(this);

    SerialUSB.println ("[Buoy] Initiating continuous transfer");
    rf->send_debug ("[Buoy] All subsystems initiated.");
    rf->send_debug ("[Buoy] Initiating continuos transfer.");
    rf->start_continuous_transfer ();
  }
}

/* vim: set filetype=arduino :  */

