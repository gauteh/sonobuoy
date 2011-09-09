/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-07
 *
 * Communication protocol to Synapse RF Wireless.
 *
 */

# ifndef RF_C
# define RF_C

# include <stdio.h>
# include <stdlib.h>

# include "buoy.h"
# include "rf.h"
# include "ad7710.h"

void rf_setup ()
{
  /* Setting up Serial interface to RF */
  RF_Serial.begin(RF_BAUDRATE);
}

/* Protocol
 *
 * Telegram consists of:
 * $Type,values,values,values*Checksum
 *
 * Type is one of:
 *  - AD    Readout from AD
 *  - GPS   GPS position and time data
 *  - STA   System status
 *  - DBG   Debug message
 *
 * After * checksum is computed as XOR of all values
 * between, and not including, $ and *. Two hexadecimal digits.
 *
 */

void rf_send_debug (char * msg)
{
  // Format
  // $DBG,[msg]*CS
  int len = strlen(msg);
  char buf[len + 10];
  sprintf(buf, "$DBG,%s*", msg);
  APPEND_CSUM (buf);

  RF_Serial.println (buf);
}

void rf_send_status ()
{
  rf_ad_message (AD_STATUS);
  rf_gps_message (GPS_STATUS);
}


void rf_ad_message (RF_AD_MESSAGE messagetype)
{
  char buf[RF_BUFLEN];

  switch (messagetype)
  {
    case AD_STATUS:
      // $AD,S,[sample rate],[value]*CS
      sprintf (buf, "$AD,S,%lu,0x%lX*", ad_sample_rate (), ad_value);
      break;

    default:
      return;
  }

  APPEND_CSUM (buf);

  RF_Serial.println (buf);
}

# define FLOAT(n) (int)n, abs((n - (int)n) * 100)

void rf_gps_message (RF_GPS_MESSAGE messagetype)
{
  char buf[RF_BUFLEN];

  switch (messagetype)
  {
    case GPS_STATUS:
      // $GPS,S,[lasttype],[telegrams received],[lasttelegram],Lat,Lon,Time,Valid*CS
      // Valid: Y = Yes, N = No
      sprintf (buf, "$GPS,S,%d,%d,%s,%c,%s,%c,%lu,%c*", gps_data.lasttype, gps_data.received, gps_data.latitude, (gps_data.north ? 'N' : 'S'), gps_data.longitude, (gps_data.east ? 'E' : 'W'), gps_data.time, (gps_data.valid ? 'Y' : 'N'));

      break;

    default:
      return;
  }

  APPEND_CSUM (buf);
  RF_Serial.println (buf);
}


byte gen_checksum (char *buf)
{
/* Generate checksum for NULL terminated string
 * (skipping first and last char) */

  byte csum = 0;
  int len = strlen(buf);

  for (int i = 1; i < (len-1); i++)
    csum = csum ^ ((byte)buf[i]);

  return csum;
}

bool test_checksum (char *buf)
{
  /* Input: String including $ and * with HEX decimal checksum
   *        to test. NULL terminated.
   */
  int len = strlen(buf);

  uint csum = 0;
  sscanf (&(buf[len-2]), F_CSUM, &csum);

  ulong tsum = 0;
  for (int i = 1; i < (len - 3); i++)
    tsum = tsum ^ (uint)buf[i];

  return tsum == csum;
}

# endif

/* vim: set filetype=arduino :  */

