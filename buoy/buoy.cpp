/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include "buoy.h"

# include <stdint.h>

# include "wirish.h"

# include "ads1282.h"
# include "rf.h"
# include "gps.h"
//# include "store.h"

namespace Buoy {
  BuoyMaster::BuoyMaster () {

  }

  void BuoyMaster::main () {
    delay (3000); // time to connect with serial before starting..
    setup ();


    while (true) {
      //gps->loop ();
      ad->loop ();
      //rf->loop ();
      //store->loop ();

      SerialUSB.println ("loop");
      delay (1000);
    }
  }

  void BuoyMaster::setup () {
    /* Configure LED */
    //pinMode (BOARD_LED_PIN, OUTPUT);
    //digitalWrite (BOARD_LED_PIN, LOW);

    //SerialUSB.begin ();

    /* Count down.. */

    for (int i = 0; i < 5; i++) {
      SerialUSB.print ("Starting soon: ");
      SerialUSB.println (i);
      delay(1000);
    }

    /* Set up devices */
    //rf    = new RF ();
    //gps   = new GPS ();
    ad    = new ADS1282 ();
    //store = new Store ();

    //rf->setup     (this);
    //gps->setup    (this);
    ad->setup     (this);
    //store->setup  (this);

    /* Start reading data continuously and writing to SD card */
    SerialUSB.println ("[Buoy] Initiating continuous transfer and write.");

    /*
    ad->start_continuous_read ();
    store->start_continuous_write ();
    */
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

