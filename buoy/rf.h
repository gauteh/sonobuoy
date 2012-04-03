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

# define rf_send_debug_f(args...)

/*
# define rf_send_debug_f(args...) \
 { uint n = sprintf(((RF*)rf)->buf, args); \
   if (n > RF_BUFLEN) ((RF*)rf)->send_debug ("[RF] [Error] DEBUG message to big. MEMORY possibly corrupted."); \
   else ((RF*)rf)->send_debug (((RF*)rf)->buf); \
   }
*/

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
      Store   *store;

      bool      isactive;
      bool      stayactive;
      uint32_t  activated; // millis () at time of activation
# define ACTIVE_TIMEOUT 60 // seconds before exiting active mode
# define STAYACTIVE_TIMEOUT (20 * 60) // seconds before exiting stay active mode

      /* Information about batch, ids or id about to be sent */
      uint32_t id;
      uint32_t ref;
      uint32_t sample;
      uint32_t length;

# define RF_SERIAL_BUFLEN 80
      char rf_buf[RF_SERIAL_BUFLEN];
      uint8_t rf_buf_pos;

      char buf[RF_BUFLEN];

      /* Incoming telegrams */
      typedef enum _RF_TELEGRAM {
        UNSPECIFIED = 0,
        UNKNOWN,
        ACTIVATE,
        DEACTIVATE,
        GETSTATUS,
        STAYACTIVE,
        GETIDS,
        GETID,
        GETLASTID,
        GETBATCH,
      } RF_TELEGRAM;

      /* Error codes */
      typedef enum _RF_ERROR {
        E_CONFIRM = 0,
        E_BADCOMMAND,
        E_UNKNOWNCOMMAND,
        E_SDUNAVAILABLE,
        E_NOSUCHID,
        E_NOSUCHREF,
        E_NOSUCHSAMPLE,
        E_NOSUCHDAT,
      } RF_ERROR;

      RF ();
      void setup (BuoyMaster *);
      void loop ();
      void parse ();

      void send_debug (const char *);
      void send_error (RF_ERROR code);

      static byte gen_checksum (const char *, bool skip = true);
      static bool test_checksum (const char *);
  };
}

/* vim: set filetype=arduino :  */

