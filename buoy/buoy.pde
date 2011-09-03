/* Author: Gaute Hope <eg@gaute.vetsj.com>
 *
 *
 */

# include "buoy.h"
# include "ad7710.c"

void setup ()
{
  /* Setting up serial link to computer */
  delay(3000);
  Serial.begin (9600);
  delay(2000);
  Serial.println ("Buoy ( version " VERSION " ) starting up..");
  Serial.println ("Gaute Hope <eg@gaute.vetsj.com> / 2011");
  
  delay (1000);
  ad_setup ();
  delay (1000);
}

void loop ()
{
  if (digitalRead(nDRDY) == LOW) {
    verbose_sample ();
  }

  delay (1000);


}

