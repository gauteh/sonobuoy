/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Interface to standard NMEA GPS.
 *
 *
 */

# pragma once

# ifndef ONLY_SPEC

# include "buoy.h"

# include <stdint.h>
# include "types.h"

namespace Buoy {
  class GPS {
# define TELEGRAM_LEN 80

# define GPS_BAUDRATE 4800

# if BBOARD == 0

# define GPS_Serial Serial1
# define GPS_SYNC_PIN 27 // Should be 5V tolerant

# elif BBOARD == 1

# define GPS_Serial Serial2
# define GPS_SYNC_PIN 2 // Should be 5V tolerant

# endif

    private:
      char gps_buf [TELEGRAM_LEN + 2];
      int  gps_buf_pos;

      void parse ();

    public:
      /*RF      * rf;*/
      ADS1282 * ad;

      /* Telegram and data structures {{{ */
      typedef enum _GPS_TELEGRAM {
        UNSPECIFIED = 0,
        UNKNOWN,
        GPRMC,
        /*
        GPGGA,
        GPGLL,
        GPGSA,
        GPGSV,
        GPVTG,
        */
      } GPS_TELEGRAM;

      /* Latest received data */
      uint16_t  received;
      GPS_TELEGRAM lasttype;
      uint32_t  time;
      uint8_t   hour;
      uint8_t   minute;
      uint8_t   second;
      uint8_t   seconds_part;
      uint8_t   day;
      uint8_t   month;
      uint16_t  year;
      bool      valid;
      char      latitude[12];
      bool      north;
      char      longitude[12];
      bool      east;

      /* For use in references */
      volatile bool ref_position_lock; // lock ref positions
      volatile char ref_latitude[12];
      volatile char ref_longitude[12];

      /*
      char      speedoverground[6];
      char      courseoverground[6]; // Ref. to True north
      */

      /*
      uint8_t   fixtype;
      uint8_t   satellites;
      uint8_t   satellites_used[12];
      uint8_t   mode1;
      uint8_t   mode2;
      */
      // }}}

      GPS ();
      void        setup (BuoyMaster *);
      void        loop  ();
# if DEBUG_VERB
      void        print_status ();
# endif

      static void sync_pulse_int ();
      void        sync_pulse ();
      void        enable_sync ();
      void        disable_sync ();

      void        update_second ();
      void        update_ref_position ();
      void        assert_time ();

      /* Timing
       *
       * Synchronization:
       * - Telegrams with UTC time information and validity is received
       *   continuously.
       *
       * - A PPS is received at the time of each second, this _must_ mean
       *   that it is synchronized to the second _after_ the last time fix
       *   received as a telegram.
       *
       * - The telegrams that are sent each second might continue to arrive
       *   during the pulse. There _should_ then already have arrived one
       *   time fix for the previous second.
       *
       * - This means that one series of telegrams might change to the next
       *   second - but _only_ after the pulse!
       *
       * - Any exact reference or timing should only be done in the PPS
       *   interrupt handler.
       *
       */
      volatile bool HAS_TIME;            // Has valid time from GPS

      volatile bool HAS_SYNC;            // Has PPS synced
      volatile bool HAS_SYNC_REFERENCE;  // Reference is set using PPS

      enum GPS_STATUS {
        NOTHING         = 0b0,
        TIME            = 0b1,
        SYNC            = 0b10,
        SYNC_REFERENCE  = 0b100,
        POSITION        = 0b1000,
      };
      /* Leap seconds:
       * Are not counted in lastseconds (unix time since epoch).
       *
       * TODO: Handle if receiver is including them in telegrams.
       */


      /* The last unix time calculated from GPS telegram, with timestamp
       * in millis (). Is also incremented by a PPS signal. */
      volatile uint64_t lastsecond;
      volatile uint64_t lastsecond_time;

      /* The latest most reliable reference for picking by AD */
      volatile uint64_t reference;
      volatile uint64_t microdelta;
      volatile uint64_t lastsync;
      volatile uint64_t lastmicros;


      /* Time to wait before manually updating reference (in case of no sync)
       *
       * Included tolerance for millis () drift.
       *
       */
# define REFERENCE_TIMEOUT 60 // [s]

      /* Maple Native Beta Crystal: 535-9721-1-ND from DigiKey */
# define TIMING_PPM  10

/* Overflow handling, the math.. {{{
 *
 * micros () overflow in about 70 minutes.
 * millis () overflow in about 50 days.
 *
 * m   = micros
 * d   = delta
 *
 * at t0:
 * ref = const.
 * d   = m0
 * t   = ref + (m0 - d)
 *
 * at t1:
 * t   = ref + (m1 - d)
 *
 * at t2:
 * m > ULONG_MAX and is clocked around
 *
 * g is modulated m
 *
 * g = m - ULONG_MAX, we _will_ catch the overflow before a second
 *                    overflow would have time to occur.
 * t = ref + (m - d)
 *   = ref + (g + ULONG_MAX - d)
 *   = ref + (g + (ULONG_MAX - d))
 *   = ref + (g - d)
 *
 * for d > g, (g - d) is clocked around backwards
 * for d > g, this means we will have to re-calculate ref before g >= d.
 *
 * The re-calculation should only be based on micros () so as to not loose
 * any resolution.
 *
 * }}} */
  };
}

# endif

/* vim: set filetype=arduino :  */

