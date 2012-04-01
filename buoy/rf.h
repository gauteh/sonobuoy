/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Communication protocol over Synapse RF Wireless.
 *
 */

# pragma once

# include <stdint.h>
# include <string.h>
# include "types.h"

namespace Buoy {
# define RF_BAUDRATE 115200
# define RF_Serial Serial3

# define RF_BUFLEN 90

/* Format for printing checksum and macro for appending checksum
 * to NULL terminated buffer with string encapsulated in $ and *.
 */
# define F_CSUM "%02hX"
# define APPEND_CSUM(buf) sprintf(&buf[strlen(buf)], F_CSUM, RF::gen_checksum(buf))

/* Macro for sending formatted debug strings, follows format of sprintf
 * will overflow if message is bigger than (RF_BUFLEN - 6 - 3)
 */

# define rf_send_debug_f(args...) \
 { uint n = sprintf(((RF*)rf)->buf, args); \
   if (n > RF_BUFLEN) ((RF*)rf)->send_debug ("[RF] [Error] DEBUG message to big."); \
   else ((RF*)rf)->send_debug (((RF*)rf)->buf); \
   }

/* Protocol
 *
 * Telegram consists of:
 * $Type,values,values,values*Checksum
 *
 * Type is one of:
 *  - AD    AD data and status messages
 *  - GPS   GPS position and time data
 *  - DBG   Debug message
 *
 * After * checksum is computed as XOR of all values
 * between, and not including, $ and *. Two hexadecimal digits.
 *
 */

  class RF {
    private:
      void *rf;
    public:
      ADS1282 *ad;
      GPS     *gps;

      char buf[RF_BUFLEN];

      typedef enum _RF_AD_MESSAGE {
        AD_STATUS = 0,
        AD_DATA_BATCH,
      } RF_AD_MESSAGE;

      typedef enum _RF_GPS_MESSAGE {
        GPS_STATUS = 0,
        GPS_POSITION,
      } RF_GPS_MESSAGE;

      RF ();
      void setup (BuoyMaster *);
      void loop ();

      /* Send data as soon as a batch is ready */
      uint8_t lastbatch;
      bool    continuous_transfer;

      /* Status is sent every second */
      uint32_t laststatus;

      void send_status ();

      void send_debug (const char *);
      void ad_message (RF_AD_MESSAGE);
      void gps_message (RF_GPS_MESSAGE);

      void start_continuous_transfer ();
      void stop_continuous_transfer ();

      static byte gen_checksum (const char *, bool skip = true);
      static bool test_checksum (const char *);
  };
}

/* vim: set filetype=arduino :  */

