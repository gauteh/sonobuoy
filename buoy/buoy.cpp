/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include "wirish.h"
# include "buoy.h"

namespace Buoy {
  BuoyMaster::BuoyMaster () {

  }

  void BuoyMaster::main () {
    delay (3000); // time to connect with serial before starting..
    setup ();


    while (true) {
      ad.loop ();
      rf.loop ();
      gps.loop ();

      delay (10);

    }
  }

  void BuoyMaster::setup () {
    /* Set up the LED to blink  */
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite (BOARD_LED_PIN, LOW);

    /* Set up devices */
    rf.setup ();
    gps.setup ();
    ad.setup ();

    rf.ad = &ad;
    rf.gps = &gps;

    gps.ad = &ad;
    gps.rf = &rf;

    ad.rf = &rf;
    ad.gps = &gps;

    SerialUSB.println ("[Buoy] Initiating continuous transfer");
    rf.send_debug ("[Buoy] All subsystems initiated.");
    rf.send_debug ("[Buoy] Initiating continuos transfer.");
    rf.continuous_transfer = true;
  }
}

/* vim: set filetype=arduino :  */

