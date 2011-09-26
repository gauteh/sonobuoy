/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-07
 *
 * Communication protocol to Synapse RF Wireless.
 *
 */

# ifndef RF_H
# define RF_H

# define RF_BAUDRATE 115200 
# define RF_Serial Serial2

# define RF_BUFLEN 150

/* Format for printing checksum and macro for appending checksum
 * to NULL terminated buffer with string encapsulated in $ and *.
 */
# define F_CSUM "%02X"
# define APPEND_CSUM(buf) sprintf(&buf[strlen(buf)], F_CSUM, \
                                  gen_checksum(buf))

typedef enum _RF_AD_MESSAGE {
  AD_STATUS = 0,
  AD_DATA_BATCH,
} RF_AD_MESSAGE;

typedef enum _RF_GPS_MESSAGE {
  GPS_STATUS = 0,
  GPS_POSITION,
} RF_GPS_MESSAGE;


byte gen_checksum (char *);
bool test_checksum (char *);
void rf_setup ();
void rf_send_debug (const char *);
void rf_send_status ();
void rf_ad_message (RF_AD_MESSAGE);
void rf_gps_message (RF_GPS_MESSAGE);

# endif

/* vim: set filetype=arduino :  */

