/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Buoy controller.
 *
 */

/* Must include all external libraries here in main sketch for build system
 * to recgonize them as a dependency. */
# include <SdFat.h>

# include "buoy.h"
# include "ad7710.h"
# include "gps.h"
# include "rf.h"
# include "store.h"


void setup ()
{
# if DIRECT_SERIAL
  Serial.begin (38400);
  Serial.println ("Serial 0 on.");
# endif

  /* Set up devices */
  rf_setup ();
  sd_setup ();
  ad_setup ();
  gps_setup ();

  /* Let devices settle */
  delay(10);
}

void loop ()
{
  static ulong laststatus = 0;

  if ((millis () - laststatus) > 500) {
    /* Send status to RF */
    rf_send_status ();

    laststatus = millis ();
  }

  ad_loop ();
  gps_loop ();
  sd_loop ();
  rf_loop ();
}

/* vim: set filetype=arduino :  */

