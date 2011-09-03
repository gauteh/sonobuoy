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
  Serial.println ("Buoy ( version " VERSION " ) starting up..");
  Serial.println ("Gaute Hope <eg@gaute.vetsj.com> / 2011");
  
  ad_setup ();

  delay(10);
  sample_performance_test ();

  sample_performance_test ();

}

void loop ()
{
  if (digitalRead(nDRDY) == LOW) {
    verbose_sample ();
  }

  delay (1000);
}

