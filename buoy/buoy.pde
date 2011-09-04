/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Buoy controller.
 *
 */

# include "buoy.h"
# include "ad7710.c"

void setup ()
{
  /* Setting up serial link to computer */
  delay(1000);
  Serial.begin (9600);
  delay(10);

  Serial.println ("[buoy] Buoy Control ( version " VERSION " ) starting up..");
  Serial.println ("[buoy] Gaute Hope <eg@gaute.vetsj.com> / 2011");

  /* Set up devices */
  ad_setup ();

  /* Let devices settle */
  delay(10);
}

void loop ()
{
  /* Print AD7710 status */
  ad_status ();

  delay (1000);
}

