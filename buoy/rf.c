/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-07
 *
 * Communication protocol and implementation to Synapse RF
 * chip.
 *
 */

# ifndef RF
# define RF

# include "buoy.h"

# define RF_BAUDRATE 38400
# define RF_Serial Serial2

# define RF_BUFLEN 1024

uint gen_checksum (char, int);
char * sgen_checksum (char, int);
bool test_checksum (char, int);
void rf_setup ();
void rf_send_status ();
void rf_ad_status ();
void rf_gps_status ();

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
 * between, not including, $ and *. Two hexadecimal digits.
 *
 */

void rf_send_status ()
{
  rf_ad_status ();
  rf_gps_status ();
}

void rf_ad_status ()
{
  int len = 4;

  // Format:
  // $AD,S,[sample rate],[lastvalue]*CS
  char buf[RF_BUFLEN];
  sprintf (buf, "$AD,R,%X, %u,*", ad_value);

  RF_Serial.println(buf);
}

void rf_gps_status ()
{
  gps_status (RF_Serial);
}


uint gen_checksum (char *buf, int len)
{
  uint csum = 0;

  for (int i = 0; i < len; i++)
    csum = csum ^ (uint)buf[i];

  return csum;
}

uint sgen_checksum (char *buf, int len, char *out)
{
  uint csum = gen_checksum (buf, len);
  sprintf (out, "%X%X", csum);
  return csum;
}

bool test_checksum (char *buf, int len)
{
  /* Input: String including $ and * with HEX decimal checksum
   *        to test.
   */

  char ccsum[2] = { buf[len-2], buf[len-1] };
  uint csum = 0;
  sscanf (ccsum, "%x%x", &csum);

  ulong tsum = 0;
  for (int i = 1; i < (len - 3); i++)
    tsum = tsum ^ (uint)buf[i];

  return tsum == csum;
}

# endif

