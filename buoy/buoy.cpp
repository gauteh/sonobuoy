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
      ad.loop ();

      delay (1000);
      if (ad.drdy) {
        SerialUSB.println ("Data ready: True");
        digitalWrite (BOARD_LED_PIN, HIGH);

        ad.acquire ();

      } else {
        SerialUSB.println ("Data ready: False");
        digitalWrite (BOARD_LED_PIN, LOW);
      }


    }
  }

  void BuoyMaster::setup () {
    /* Set up the LED to blink  */
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite (BOARD_LED_PIN, LOW);

    /* Set up devices */
    ad.setup ();

  }

  /* Miscellaneous functions */

  /* shiftIn from Arduino / Wiring libs - completes shiftOut from Wirish */
  uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    uint8_t value = 0;
    uint8_t i;

    for (i = 0; i < 8; ++i) {
      digitalWrite(clockPin, HIGH);
      if (bitOrder == LSBFIRST)
        value |= digitalRead(dataPin) << i;
      else
        value |= digitalRead(dataPin) << (7 - i);
      digitalWrite(clockPin, LOW);
    }
    return value;
  }
}

/* vim: set filetype=arduino :  */

