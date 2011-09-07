/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-07
 *
 * Communication protocol and to Synapse RF Wireless.
 *
 */

# ifndef RF_C
# define RF_C

# include "buoy.h"
# include "rf.h"

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
 *
 * After * checksum is computed as XOR of all values
 * between, and not including, $ and *. Two hexadecimal digits.
 *
 */

void rf_send_status ()
{
  rf_ad_message (AD_STATUS);
  rf_gps_status ();
}


void rf_ad_message (RF_AD_MESSAGE messagetype)
{
  char buf[RF_BUFLEN];

  switch (messagetype)
  {
    case AD_STATUS:
      // $AD,S,[sample rate],[value]*CS
      sprintf (buf, "$AD,R,%lu,0x%lX,*", ad_sample_rate (), ad_value);

      break;
    default: return;
  }

  APPEND_CSUM(buf);

  RF_Serial.println(buf);
}

void rf_gps_status ()
{
  gps_status (RF_Serial);
}


uint gen_checksum (char *buf)
{
/* Generate checksum for NULL terminated string
 * (skipping first and last char) */

  uint csum = 0;
  int len = strlen(buf);

  for (int i = 1; i < (len-1); i++)
    csum = csum ^ (uint)buf[i];

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

