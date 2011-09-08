/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Buoy controller.
 *
 */

# include <stdlib.h>
# include <stdio.h>

# include "buoy.h"
# include "ad7710.c"
# include "gps.c"
# include "rf.c"

ulong laststatus = 0;

void setup ()
{
  /* Set up devices */
  ad_setup ();
  gps_setup ();
  rf_setup ();

  /* Let devices settle */
  delay(10);
}

void loop ()
{

  if ((millis () - laststatus) > 1000) {
    /* Send status to RF */
    rf_send_status ();

    laststatus = millis ();
  }


  gps_loop ();

  delay(1);
}

/* vim: set filetype=arduino :  */

