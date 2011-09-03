/* Author: Gaute Hope <eg@gaute.vetsj.com>
 *
 *
 */

# include "buoy.h"
# include "ad7710.c"

void setup ()
{
  /* Setting up serial link to computer */
  Serial.begin (9600);

  Serial.println ("Buoy (" + str(VERSION) + ") starting up..");
}

void loop ()
{


}

