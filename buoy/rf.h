/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-07
 *
 * Communication protocol and to Synapse RF Wireless.
 *
 */

# ifndef RF_H
# define RF_H

# define RF_BAUDRATE 38400
# define RF_Serial Serial2

# define RF_BUFLEN 1024

/* Format for printing checksum and macro for appending checksum
 * to NULL terminated buffer with string encapsulated in $ and *.
 */
# define F_CSUM "%02lX"
# define APPEND_CSUM(buf) sprintf(&buf[strlen(buf)], F_CSUM, \
                                  gen_checksum(buf))

typedef enum _RF_AD_MESSAGE {
  AD_STATUS = 0,
  AD_SAMPLE,
} RF_AD_MESSAGE;

uint gen_checksum (char *);
bool test_checksum (char *);
void rf_setup ();
void rf_send_status ();
void rf_ad_message (RF_AD_MESSAGE);
void rf_gps_status ();

# endif

/* vim: set filetype=arduino :  */

