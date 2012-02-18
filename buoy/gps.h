/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Interface to standard NMEA GPS.
 *
 *
 */

# pragma once

# ifndef ONLY_SPEC

# include <stdint.h>
# include "types.h"

namespace Buoy {
  class GPS {
# define TELEGRAM_LEN 80

# define GPS_BAUDRATE 4800
# define GPS_Serial Serial1

# define GPS_SYNC_PIN 27 // Should be 5V tolerant

    private:
      char gps_buf [TELEGRAM_LEN + 2];
      int  gps_buf_pos;

      /* Keep track of last sync pulse */
      volatile uint32_t lastsync;
      volatile uint8_t  referencerolled; // Pass debug message from main loop

      void parse ();

    public:
      RF      * rf;
      ADS1282 * ad;

      /* Telegram and data structures {{{ */
      typedef enum _GPS_TELEGRAM {
        UNSPECIFIED = 0,
        UNKNOWN,
        GPRMC,
        GPGGA,
        GPGLL,
        GPGSA,
        GPGSV,
        GPVTG,
      } GPS_TELEGRAM;

      typedef struct _GPS_DATA {
        GPS_TELEGRAM lasttype;
        char    lasttelegram[TELEGRAM_LEN];
        int     received; /* Received telegrams */
        bool    valid;
        int     fixtype;

        int     satellites;
        int     satellites_used[12];
        uint8_t    mode1;
        uint8_t    mode2;

        char    latitude[12];
        bool    north;    /* true = Latitude is north aligned, false = south */
        char    longitude[12];
        bool    east;     /* true = Longitude is east aligned, false = south */

        uint32_t   time;
        int     hour;
        int     minute;
        int     second;
        int     seconds_part;
        int     day;
        int     month;
        int     year;

        char    speedoverground[6];
        char    courseoverground[6]; /* True north */
      } GPS_DATA; // }}}

      GPS_DATA gps_data;

      GPS ();
      void        setup (BuoyMaster *);
      void        loop ();

      static void sync_pulse_int ();
      void        sync_pulse ();
      void        enable_sync ();
      void        disable_sync ();

      void        update_second ();


      /* Timing */
      bool HAS_TIME;                     // Has valid time from GPS

      volatile bool HAS_SYNC;            // Has PPS synced
      volatile bool HAS_SYNC_REFERENCE;  // Reference is set using PPS
      volatile bool IN_OVERFLOW;         // micros () is overflowed

      /* Leap seconds:
       * Are not counted in lastseconds (unix time since epoch).
       *
       * TODO: Handle if receiver is including them in telegrams.
       */


      /* The last unix time calculated from GPS telegram, with timestamp
       * in millis (). Is also incremented by a PPS signal. */
      uint64_t lastsecond;
      uint64_t lastsecond_time;

      /* The latest most reliable reference for picking by AD */
      uint64_t reference;
      uint64_t microdelta;

# define ULONG_MAX ((2^32) - 1)

/* Overflow handling, the math.. {{{
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
 * g = m - ULONG_MAX, we _will_ catch overrun/modulation before a second
 *                    modulation would have time to occur.
 * t = ref + (m - d)
 *   = ref + (g + ULONG_MAX - d)
 *   = ref + (g + (ULONG_MAX - d))
 *
 * for d > g, this means we will have to recalculate ref before g >= d.
 *
 * }}} */
  };
}

# endif

/* vim: set filetype=arduino :  */

