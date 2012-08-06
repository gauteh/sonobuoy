/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Communication protocol over Synapse RF Wireless.
 *
 */

# pragma once

# include "buoy.h"
# include "types.h"

# include <stdint.h>

namespace Buoy {
# define RF_BAUDRATE 115200
# if BBOARD == 0

# define RF_Serial Serial3

# elif BBOARD == 1

# define RF_Serial Serial1

# endif

//# define RF_BUFLEN 90

/* Format for printing checksum and macro for appending checksum
 * to NULL terminated buffer with string encapsulated in $ and *.
 */
# define F_CSUM "%02hX"
//# define APPEND_CSUM(buf) sprintf(&buf[strlen(buf)], F_CSUM, RF::gen_checksum(buf))

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

      /* Information about batch, ids or id about to be sent */
      uint32_t id;
      uint32_t ref;
      uint32_t sample;
      uint32_t length;

      /* sane values for above arguments */
      # define MAX_SANE_REF     MAX_REFERENCES
      # define MAX_SANE_ID      MAXID
      # define MAX_SANE_SAMPLE  MAX_SAMPLES_PER_FILE
      # define MAX_SANE_LENGTH  (50 * BATCH_LENGTH)

# define RF_SERIAL_BUFLEN 80
      char rf_buf[RF_SERIAL_BUFLEN];
      uint8_t rf_buf_pos;

      /* Incoming telegrams */
      typedef enum _RF_TELEGRAM {
        UNSPECIFIED = 0,
        UNKNOWN,
        GETSTATUS,
        GETIDS,
        GETID,
        GETLASTID,
        GETBATCH,
        GETINFO
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
        E_BADDAT,
        E_SDLAG,
        E_MAXIDREACHED,
      } RF_ERROR;

      RF ();
      void setup (BuoyMaster *);
      void loop ();
      void parse ();
      void simple_parser (RF_TELEGRAM);

      void send_debug (const char *);
      void send_error (RF_ERROR code);
  };
}

/* vim: set filetype=arduino :  */

