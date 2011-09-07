/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Buoy controller.
 *
 */

# include "buoy.h"
# include "ad7710.c"
# include "linx_rxm_gps.c"

ulong laststatus = 0;

void setup ()
{
  /* Setting up serial link to computer */
  delay(1000);
  Serial.begin (9600);
  delay(10);

  /* Setting up serial link to RF200 */
  Serial2.begin (38400);

  Serial.println ("[Buoy] Buoy Control ( version " VERSION " ) starting up..");
  Serial.println ("[Buoy] by Gaute Hope <eg@gaute.vetsj.com> / <gaute.hope@student.uib.no>  (2011)");

  /* Set up devices */
  ad_setup ();
  gps_setup ();

  /* Let devices settle */
  delay(10);
}

void loop ()
{

  if ((millis () - laststatus) > 1000) {
    /* Print AD7710 status */
    ad_status (Serial);
    ad_status (Serial2);

    gps_status (Serial);
    gps_status (Serial2);
    
    laststatus = millis ();
  }

  gps_loop ();

  delay(1);
}

/* vim: set filetype=arduino :  */

