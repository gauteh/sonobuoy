/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include "wirish.h"
# include "buoy.h"
# include "ads1282.h"

namespace Buoy {
  BuoyMaster::BuoyMaster () {

  }

  void BuoyMaster::main () {
    setup ();

    while (true) {
      toggleLED();
      delay(1000);
      SerialUSB.println("Hello!");
    }
  }

  void BuoyMaster::setup () {
    /* Set up the LED to blink  */
    pinMode(BOARD_LED_PIN, OUTPUT);

    /* Set up devices */
    ad.setup ();

  }
}

/* vim: set filetype=arduino :  */

