/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-07
 *
 * Communication protocol and to Synapse RF Wireless.
 *
 */

# ifndef RF
# define RF

# include "buoy.h"

# define RF_BAUDRATE 38400
# define RF_Serial Serial2

# define RF_BUFLEN 1024

/* Format for printing checksum and macro for appending checksum
 * to NULL terminated buffer with string encapsulated in $ and *.
 */
# define F_CSUM "%lX02"
# define APPEND_CSUM(buf) sprintf(&buf[strlen(buf)], F_CSUM, \
                                  gen_checksum(buf))

uint gen_checksum (char *);
bool test_checksum (char *);
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
 * between, and not including, $ and *. Two hexadecimal digits.
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
  // $AD,S,[sample rate],[value]*CS
  char buf[RF_BUFLEN];
  sprintf (buf, "$AD,R,%lu,%lX,*", ad_sample_rate (), ad_value);
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

  char ccsum[2] = { buf[len-2], buf[len-1] };
  uint csum = 0;
  sscanf (ccsum, "%x%x", &csum);

  ulong tsum = 0;
  for (int i = 1; i < (len - 3); i++)
    tsum = tsum ^ (uint)buf[i];

  return tsum == csum;
}

# endif

