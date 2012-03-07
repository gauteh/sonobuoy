/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Interface to standard NMEA GPS.
 *
 *
 */

# include <stdio.h>
# include <string.h>
# include "buoy.h"

# include "gps.h"
# include "ads1282.h"
# include "rf.h"

using namespace std;

namespace Buoy {
  GPS::GPS () {
    HAS_TIME = false;
    HAS_SYNC = false;
    HAS_SYNC_REFERENCE = false;

    lastsecond      = 0;
    lastsecond_time = 0;

    reference       = 0;
    microdelta      = 0;

    lastmicros      = micros ();
    lastsync        = 0;
  }

  void GPS::setup (BuoyMaster *b) {
    rf = b->rf;
    ad = b->ad;

    gps_buf[0] = 0;
    gps_buf_pos = 0;

    memset (&gps_data, 0, sizeof(gps_data));

    GPS_Serial.begin (GPS_BAUDRATE);

    pinMode (GPS_SYNC_PIN, INPUT_PULLDOWN);
    enable_sync ();

    rf->send_debug ("[GPS] GPS subsystem initiated.");
  }

  void GPS::sync_pulse_int () {
    bu->gps->sync_pulse ();
  }

  void GPS::sync_pulse () {
    if (HAS_TIME) {
      /* If a PPS pulse is sent it must be for the second after the last
       * received as a telegram */
      lastsecond++;

      /* New available reference */
      reference  = lastsecond;

      /* Update microdelta */
      microdelta = micros ();

      /* Reset last sync counter */
      lastsync   = millis ();

      HAS_SYNC            = true;
      HAS_SYNC_REFERENCE  = true;
    }
  }

  void GPS::assert_time () {
    /* Check state of timing and PPS */

    /* We have a tolerance of 1 millisecond to catch sync loss */
# define LOST_SYNC 1001

    /* Check if we still have sync */
    if ((millis () - lastsync) > LOST_SYNC) {
      HAS_SYNC = false;

      /* Check if we need to manually update reference:
       *
       * If micros () has overflowed since last microdelta and reference,
       * a new reference and microdelta must be set before it reaches the second
       * overflow.
       *
       * By ensuring that the reference is updated withing the roll over time for
       * micros () this should not happen.
       *
       */

      if (( (millis() - lastsync) > (REFERENCE_TIMEOUT * 1000) ) && ( (lastsecond - reference) > REFERENCE_TIMEOUT )) {
        HAS_SYNC_REFERENCE = false;

        /* Un-reliable, using time telegram */
        reference  = lastsecond;
        microdelta = micros () + ((millis () - lastsecond_time) * 1000);
      }
    }
  }

  void GPS::update_second () {
    /* Calculate Unix time from UTC {{{ */

# define SECONDS_PER_DAY 86400L
# define LEAP_YEAR(x) !!(!((1970 + x) % 4) && ( ((1970 + x) % 100) || !((1970 + x) % 400) ))

    uint32_t year = (2000 + gps_data.year) - 1970; // Offset 1970 (unix epoch)

    uint64_t newsecond = year * 365 * SECONDS_PER_DAY;

# define LEAP_YEARS_BEFORE_1970 ((1970 / 4) - (1970 / 100) + (1970 / 400))
    /* Add a day of seconds for each leap year except this */
    newsecond += (( (1970 + (year-1)) /   4 )
               - (  (1970 + (year-1)) / 100 )
               + (  (1970 + (year-1)) / 400 )
               - LEAP_YEARS_BEFORE_1970     ) * SECONDS_PER_DAY;

    const int monthdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 1; i < gps_data.month; i++) {
      if ((i == 2) && LEAP_YEAR(year))
        newsecond += SECONDS_PER_DAY * 29;
      else
        newsecond += SECONDS_PER_DAY * monthdays[i - 1];
    }

    newsecond += SECONDS_PER_DAY * (gps_data.day - 1);
    newsecond += (uint32_t)gps_data.hour   * 60 * 60;
    newsecond += (uint32_t)gps_data.minute * 60;
    newsecond += (uint32_t)gps_data.second;

    gps_data.time  = gps_data.hour    * 1e4;
    gps_data.time += gps_data.minute  * 1e2;
    gps_data.time += gps_data.second;

    /* Make sure lastsync is not changed while working */
    disable_sync ();

    /* Update last second if we don't have a valid time from before
     * or there has been an increment in seconds (wheter the new second is
     * valid or not). */
    if (!HAS_TIME || newsecond > lastsecond) {
      lastsecond_time = millis ();
      lastsecond      = newsecond;
    }

    HAS_TIME = gps_data.valid;

    /* Set first time reference */
    if (reference == 0) {
# if DIRECT_SERIAL
      SerialUSB.println ("[GPS] Setting first-time reference.");
# endif
      rf->send_debug ("[GPS] Setting first-time reference.");

      reference   = lastsecond;
      microdelta  = micros ();  // unreliable
      HAS_SYNC_REFERENCE  = false;

      // Pick for ADS1282
      if (!ad->continuous_read) {
        ad->references[ad->batch] = reference;
        ad->microdeltas[ad->batch] = microdelta;
        ad->reference_status[ad->batch] = (HAS_TIME & GPS::TIME) |
                                          (HAS_SYNC & GPS::SYNC) |
                                          (HAS_SYNC_REFERENCE & GPS::SYNC_REFERENCE);
      }
    }

    enable_sync ();
    // }}}
  }

  void GPS::loop () {
    /* Handle incoming GPS telegrams on serial line (non-blocking) {{{
     *
     *
     * States:
     * 0 = Waiting for start of telegram $
     * 1 = Receiving (between $ and *)
     * 2 = Waiting for Checksum digit 1
     * 3 = Waiting for Checksum digit 2
     */
    static int state = 0;

    int ca = GPS_Serial.available ();

    while (ca > 0) {
      char c = (char)GPS_Serial.read ();

      switch (state)
      {
        case 0:
          if (c == '$') {
            gps_buf[0] = '$';
            gps_buf_pos = 1;
            state = 1;
          }
          break;

        case 2:
          state = 3;
        case 1:
          if (c == '*') state = 2;

          gps_buf[gps_buf_pos] = c;
          gps_buf_pos++;
          break;

        case 3:
          gps_buf[gps_buf_pos] = c;
          gps_buf_pos++;
          gps_buf[gps_buf_pos] = 0;

          parse (); // Complete telegram received
          state = 0;
          break;

        /* Should not be reached. */
        default:
          state = 0;
          break;
      }

      ca--;
    }

    /* }}} Done telegram handler */

  }

  void GPS::parse ()
  {
    /* GPS parser (non-blocking) {{{
     *
     *
     */

    gps_data.received++;

    GPS_TELEGRAM type = UNSPECIFIED;
    int  tokeni = 0;
    int  len = gps_buf_pos; // Excluding NULL terminator

    for (int i = 0; i < (len+1); i++)
      gps_data.lasttelegram[i] = gps_buf[i];


    /* Test checksum before parsing */
    if (!rf->test_checksum (gps_buf)) return;

    /* Update time, should be set in case time data has been received */
    bool doseconds = false;

    /* Parse */
    int i = 0;
    while (i < len)
    {
      /*
      uint32_t ltmp = 0;
      uint32_t remainder = 0;
      */

      char token[80]; // Max length of token
      int j = 0;
      /* Get next token */
      while ((gps_buf[i] != ',' && gps_buf[i] != '*') && i < len) {
        token[j] = gps_buf[i];

        i++;
        j++;
      }
      i++; /* Skip delimiter */

      token[j] = 0;

      if (i < len) {
        if (tokeni == 0) {
          /* Determine telegram type */
          if (strcmp(token, "$GPRMC") == 0)
            type = GPRMC;
          else if (strcmp(token, "$GPGGA") == 0)
            type = GPGGA;
          else if (strcmp(token, "$GPGLL") == 0)
            type = GPGLL;
          else if (strcmp(token, "$GPGSA") == 0)
            type = GPGSA;
          else if (strcmp(token, "$GPGSV") == 0)
            type = GPGSV;
          else if (strcmp(token, "$GPVTG") == 0)
            type = GPVTG;
          else {
            /* Cancel parsing */
            type = UNKNOWN;
            return;
          }
          gps_data.lasttype = type;

        } else {
          switch (type)
          {
            // GPRMC {{{
            case GPRMC:
              switch (tokeni)
              {
                case 1:
                  {
                    int r = sscanf (token, "%02d%02d%02d.%d", &(gps_data.hour), &(gps_data.minute), &(gps_data.second), &(gps_data.seconds_part));

                    // Update if we got all values
                    doseconds = (r == 4) && gps_data.day > 0;
                  }
                  break;

                case 2:
                  gps_data.valid = (token[0] == 'A');
                  break;

                case 3:
                  strcpy (gps_data.latitude, token);
                  break;

                case 4:
                  gps_data.north =  (token[0] == 'N');
                  break;

                case 5:
                  strcpy (gps_data.longitude, token);
                  break;

                case 6:
                  gps_data.east = (token[0] == 'E');
                  break;

                case 7:
                  strcpy (gps_data.speedoverground, token);
                  break;

                case 8:
                  strcpy (gps_data.courseoverground, token);
                  break;

                case 9:
                  {
                    int r = sscanf (token, "%02d%02d%02d", &(gps_data.day), &(gps_data.month), &(gps_data.year));
                    // Update if we got all values, and got time
                    doseconds = (r == 3) && gps_data.day > 0;
                  }
                  break;

                case 10: /* Magnetic declination not supported by device */

                default:
                  break;
              }
              break;
              // }}}

            // GPGGA {{{
            case GPGGA:
              switch (tokeni)
              {
                case 1:
                  {
                    int r = sscanf (token, "%02d%02d%02d.%d", &(gps_data.hour), &(gps_data.minute), &(gps_data.second), &(gps_data.seconds_part));

                    // Update if we got all values
                    doseconds = (r == 4) && gps_data.day > 0;
                  }
                  break;

                case 2:
                  strcpy (gps_data.latitude, token);
                  break;

                case 3:
                  gps_data.north =  (token[0] == 'N');
                  break;

                case 4:
                  strcpy (gps_data.longitude, token);
                  break;

                case 5:
                  gps_data.east = (token[0] == 'E');
                  break;

                case 6:
                  /* 0 = Fix not valid
                   * 1 = GPS SPS mode, fix valid
                   * 2 = Differential GPS SPS mode, fix valid
                   * 3 = GPS PPS mode, fix valid
                   */
                  gps_data.valid = (token[0] >= '1');
                  gps_data.fixtype = (token[0] - '0');
                  break;

                case 7:
                  sscanf (token, "%d", &(gps_data.satellites));
                  break;
                case 8:  break; /* Discard HDOP */
                case 9:  break; /* Discard MSL altitude */
                case 10: break; /* Discard unit for ^^ */
                case 11: break; /* No support for Geoid separation */
                case 12: break; /* Discard unit for ^^ */
                case 13: break; /* Discard age of diff correction */
                case 14: break; /* Discard Diff. Ref. station ID */

                default:
                  break;

              }
              break;
            // }}}

            // GPGLL {{{
            case GPGLL:
              switch (tokeni)
              {
                case 1:
                  strcpy (gps_data.latitude, token);
                  break;

                case 2:
                  gps_data.north =  (token[0] == 'N');
                  break;

                case 3:
                  strcpy (gps_data.longitude, token);
                  break;

                case 4:
                  gps_data.east = (token[0] == 'E');
                  break;

                case 5:
                  {
                    int r = sscanf (token, "%02d%02d%02d.%d", &(gps_data.hour), &(gps_data.minute), &(gps_data.second), &(gps_data.seconds_part));
                    doseconds = (r == 4) && gps_data.day > 0;
                  }
                  break;

                case 6:
                  gps_data.valid = (token[0] == 'A');
                  break;

                default: break;
              }
              break;
            // }}}

            // GPGSA {{{
            case GPGSA:
              switch (tokeni)
              {
                case 1:
                  /* M = Manual forced to operate in 2D or 3D mode
                   * A = Automatic-allowed to automatically switch 2D/3D
                   */
                  gps_data.mode1 = (token[0] == 'M' ? 0 : 1);
                  break;

                case 2:
                  /* 0 => 1 = Fix not available
                   * 1 => 2 = 2D
                   * 2 => 3 = 3D
                   */
                  gps_data.mode2 = (token[0] - '1');
                  gps_data.valid = (token[0] > '1');
                  break;

                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                {
                  static int s = 0;
                  sscanf (token, "%d", &(gps_data.satellites_used[s]));
                  s++;

                  if (s >= 12) s = 0;
                }
                break;

                /* Discard:
                 * - PDOP
                 * - HDOP
                 * - VDOP
                 */
                default: break;
              }
              break;
            // }}}

            // GPGSV {{{
            case GPGSV:
            /* TODO: Handle multiple messages.. simply ignoring, could result
             * in wrong values for satellite count */
              {
                switch (tokeni)
                {
                  case 1:
                    break; /* Discard number of messages */

                  case 2:
                    break; /* Discard message number */

                  case 3:
                    sscanf (token, "%d", &(gps_data.satellites));

                  /* Discard the individual satellite data */

                  default:
                    break;
                }

              }
              break;
            // }}}

            default:
              /* Having reached here on an unknown or unspecified telegram
               * parsing is cancelled. */
              return;
          }
        }
      } else {
        /* Last token: Check sum */
      }
      tokeni++;
    }

    if (doseconds)
      update_second ();

    /* Done parser }}} */
  }

  void inline GPS::enable_sync () {
    attachInterrupt (GPS_SYNC_PIN, &(GPS::sync_pulse_int), FALLING);
  }
  void inline GPS::disable_sync () {
    detachInterrupt (GPS_SYNC_PIN);
  }
}

/* vim: set filetype=arduino :  */

