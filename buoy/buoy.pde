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
  Serial.println ("[BUOY] Buoy Control ( version " VERSION " ) starting up..");
  Serial.println ("[BUOY] Gaute Hope <eg@gaute.vetsj.com> / 2011");
  
  ad_setup ();

  delay(10);
}

void loop ()
{
  Serial.print ("[BUOY] [Status] AD sample rate: ");

  ulong srate = (ad_samples * 1000) / (millis() - ad_start);
  Serial.print (srate);
  Serial.print (" [Hz], last value: ");
  Serial.println (ad_value, HEX);

  /* Reset sample rate counter */
  ad_samples = 0;
  ad_start   = millis ();

  delay (1000);
}

