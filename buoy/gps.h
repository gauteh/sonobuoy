/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-09
 *
 * Interface to standard NMEA GPS.
 *
 *
 */

# ifndef GPS_H
# define GPS_H

# define TELEGRAM_LEN 80

# define GPS_BAUDRATE 4800
# define GPS_Serial Serial1

# define GPS_SYNC_PIN 3
# define GPS_SYNC_INTERRUPT 1

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
  byte    mode1;
  byte    mode2;

  char    latitude[12];
  bool    north;    /* true = Latitude is north aligned, false = south */
  char    longitude[12];
  bool    east;     /* true = Longitude is east aligned, false = south */

  ulong   time;
  int     hour;
  int     minute;
  int     second;
  int     seconds_part;
  int     day;
  int     month;
  int     year;

  char    speedoverground[6];
  char    courseoverground[6]; /* True north */
} GPS_DATA;

extern GPS_DATA gps_data;

/* Timing */
# define LEAP_SECONDS 19 // as of 2011
extern bool HAS_LEAP_SECONDS;             // Has received leap seconds inf.
extern bool HAS_TIME;                     // Has valid time from GPS
extern volatile bool HAS_SYNC;            // Has PPS synced
extern volatile bool HAS_SYNC_REFERENCE;  // Reference is set using PPS
extern volatile bool IN_OVERFLOW;         // micros () is overflowed

/* Current basis to calculate microsecond timestamp from
 *
 * Needs to be logged as reference for following time stamps in data.
 *
 * ROLL_REFERENCE specifies how many seconds between each reference point
 * referencesecond to current.
 *
 */
extern volatile ulong referencesecond;
# define ROLL_REFERENCE 60

/* Last second received from GPS , the next pulse should indicate +1 second.
 * UTC (HAS_LEAP_SECONDS indicate wether this includes leap seconds)
 *
 * This is updated even without valid data and can only be trusted if
 * data.valid is set and gps_update_second has been run.
 *
 */
extern volatile ulong lastsecond;

/* The time in microseconds between Arduino micros() clock and referencesecond
 *
 * Synchronized with pulse from GPS.
 *
 */
extern volatile ulong microdelta;

/*
 * For detecting overflow: lastmicros is result of last micros ()
 *                         call in ad_drdy ()
 *
 * TODO: Check if we count the first step (0) correctly when in overflow.
 *
 */
extern volatile ulong lastmicros;

/* Get time related to reference */
# define TIME_FROM_REFERENCE (!IN_OVERFLOW ? (micros() - microdelta) : (micros () + (ULONG_MAX - microdelta)))
# define CHECK_FOR_OVERFLOW() (IN_OVERFLOW = (micros () < lastmicros))

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

void gps_setup ();
void gps_loop ();
void gps_sync_pulse ();
void gps_roll_reference ();
void gps_update_second ();

# endif
