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


void rf_send_status ()
{
  //rf_ad_message (AD_STATUS);
  //rf_gps_message (GPS_STATUS);
  rf_ad_message (AD_DATA_BATCH);

  /*
  char buf[RF_BUFLEN];
  sprintf(buf, "AD queue postion: %d", ad_qposition);
  rf_send_debug (buf);
  */
}

void rf_send_debug (const char * msg)
{
  /* Format:
   * $DBG,[msg]*CS
   *
   */

  char buf[RF_BUFLEN];
  sprintf(buf, "$DBG,%s*", msg);
  APPEND_CSUM (buf);

  RF_Serial.println (buf);
}

void rf_ad_message (RF_AD_MESSAGE messagetype)
{
  char buf[RF_BUFLEN];

  switch (messagetype)
  {
    case AD_STATUS:
      // $AD,S,[sample rate],[value]*CS
      sprintf (buf, "$AD,S,%lu,0x%02X%02X%02X*", ad_sample_rate (), ad_value[0], ad_value[1], ad_value[2]);
      APPEND_CSUM (buf);

      RF_Serial.println (buf);

      break;

    case AD_DATA_BATCH:
      /* Send AD_DATA_BATCH_LEN samples
       *
       * The RF200 (AtMega128) can hold strings of maximum 126 bytes.
       *
       */
      # define AD_DATA_BATCH_LEN 26

      /* Format:

       * 1. Initiate binary data stream:

       $AD,D,[k = number of samples],[time of first s]*CC

       * 2. Send one $

       * 3. Send k number of samples: 4 bytes * k

       * 4. Send end of data with checksum

       */
      {
        int l = ad_qposition;

        int n = sprintf (buf, "$AD,D,%d,%d*", AD_DATA_BATCH_LEN, ad_time[l - AD_DATA_BATCH_LEN]);
        APPEND_CSUM (buf);
        RF_Serial.println (buf);

        delayMicroseconds (100);

        byte csum = 0;

        /* Write '$' to signal start of binary data */
        RF_Serial.write ('$');

        sample lasts;
        sample s;

        for (int i = 0; i < AD_DATA_BATCH_LEN; i++)
        {
          memcpy (s, (const void *) ad_queue[l - AD_DATA_BATCH_LEN + i], 3);
          /* MSB first (big endian), means concatenating bytes on RX will
           * result in LSB first; little endian. */
          RF_Serial.write (s, 3);

          csum = csum ^ s[0];
          csum = csum ^ s[1];
          csum = csum ^ s[2];

          memcpy (lasts, s, 3);

          delayMicroseconds (100);
        }

        /* Send end of data with Checksum */
        sprintf (buf, "$AD,DE," F_CSUM "*", csum);
        APPEND_CSUM (buf);
        RF_Serial.println (buf);
        delayMicroseconds (100);

/*
        sprintf(buf, "AD last sent val: 0x%02X%02X%02X", lasts[0], lasts[1], lasts[2]);
        rf_send_debug (buf);
        delayMicroseconds (100);
        */
      }
      break;

    default:
      return;
  }
}

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

