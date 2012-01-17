/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-05
 *
 * Interface to GlobalSat EM-411 with sync pulse.
 *
 * Appears to be based on SirfStar III.
 *
 * We assume we will receive telegrams within each second wether
 * there is valid data or not.
 *
 * With no valid data, the time and date figured out by the GPS is used anyway.
 *
 */

# ifndef GPS
# define GPS


# include "buoy.h"
# include "rf.h"
# include "gps.h"
# include "store.h"

char gps_buf[TELEGRAM_LEN + 2];
int  gps_buf_pos   = 0;

/* Structure holding the latest GPS data */
GPS_DATA gps_data;

void gps_setup ()
{
  GPS_Serial.begin (GPS_BAUDRATE);

  gps_buf[0] = 0;
  gps_buf_pos = 0;

  memset (&gps_data, 0, sizeof(gps_data));

  pinMode (GPS_SYNC_PIN, INPUT);
  attachInterrupt (GPS_SYNC_INTERRUPT, gps_sync_pulse, RISING);
}


bool HAS_LEAP_SECONDS             = false;
bool HAS_TIME                     = false;
volatile bool HAS_SYNC            = false;
volatile bool HAS_SYNC_REFERENCE  = false;
volatile bool IN_OVERFLOW         = false;

volatile ulong referencesecond = 0;
volatile ulong lastsecond      = 0;
volatile ulong microdelta      = 0;

volatile ulong lastmicros      = 0;

/* Keep track of last sync pulse */
volatile ulong lastsync        = 0;

void gps_sync_pulse ()
{
  /* Assume data is valid if we have PPS */

  /* Synchronize time stamp clock */
  HAS_SYNC = true;
  lastsync = millis ();

  /* Last second should be received every second */
  lastsecond++;

  /* Update microdelta offset */
  microdelta = micros () - (1e6 * (lastsecond - referencesecond));

  /* Is reset because of new microdelta calculation */
  IN_OVERFLOW = false;

  /* Reset overrun handling */
  lastmicros = micros ();

  /* Check wether to update reference second */
  if ((lastsecond - referencesecond) > ROLL_REFERENCE)
    gps_roll_reference ();

  /* Update reference in case reference has been set using local clock */
  if (!HAS_SYNC_REFERENCE)
    gps_roll_reference ();
}

void gps_roll_reference ()
{
  /* Change referencesecond to latest */
  rf_send_debug ("Roll reference.");
  microdelta = microdelta - (1e6 * (lastsecond - referencesecond));
  referencesecond = lastsecond;
  update_reference = true; // Signal to store that new reference is available
  update_reference_qposition = ad_qposition;
  HAS_SYNC_REFERENCE = true;
}

void gps_update_second ()
{
  /* Calculate Unix time from UTC {{{
   * Based on makeTime () as of 2011-10-05 from:
   * http://www.arduino.cc/playground/Code/Time */

# define SECONDS_PER_DAY 86400L
# define LEAP_YEAR(x) (!((1970 + x) % 4) && ( ((1970 + x) % 100) || !((1970 + x) % 400) ))

  ulong year = (2000 + gps_data.year) - 1970; // Offset 1970

  lastsecond = year * 365 * SECONDS_PER_DAY;

  /* Add a day for each leap year */
  for (int i = 0; i < year; i++)
    if (LEAP_YEAR(i))
      lastsecond += SECONDS_PER_DAY;

  const int monthdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  for (int i = 1; i < gps_data.month; i++) {
    if ((i == 2) && LEAP_YEAR(year))
      lastsecond += SECONDS_PER_DAY * 29;
    else
      lastsecond += SECONDS_PER_DAY * monthdays[i - 1];
  }

  lastsecond += SECONDS_PER_DAY * (gps_data.day - 1);
  lastsecond += 60 * 60 * (ulong)gps_data.hour;
  lastsecond += 60 * (ulong)gps_data.minute;
  lastsecond += (ulong)gps_data.second;


  gps_data.time  = gps_data.hour * 1e4;
  gps_data.time += gps_data.minute * 1e2;
  gps_data.time += gps_data.second;

  // }}}

  /* Setting first time reference */
  if (referencesecond == 0) {
    gps_roll_reference ();
    HAS_SYNC_REFERENCE = false;
  }
}

void gps_parse ()
{
  /* GPS parser {{{
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
  if (!test_checksum (gps_buf)) return;

  /* Update time, should be set in case time data has been received */
  bool doseconds = false;

  /* Parse */
  int i = 0;
  while (i < len)
  {
    ulong ltmp = 0;
    ulong remainder = 0;

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
                  int r = sscanf (token, "%02d%02d%02d.%lu", &(gps_data.hour), &(gps_data.minute), &(gps_data.second), &(gps_data.seconds_part));

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
                  int r = sscanf (token, "%02d%02d%02d.%lu", &(gps_data.hour), &(gps_data.minute), &(gps_data.second), &(gps_data.seconds_part));

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
                  int r = sscanf (token, "%02d%02d%02d.%lu", &(gps_data.hour), &(gps_data.minute), &(gps_data.second), &(gps_data.seconds_part));
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
    gps_update_second ();

  /* Done parser }}} */
}

void gps_loop ()
{
  /* Check if we are closing second overflow, fix by
   * setting new reference using internal clock.
   *
   * This can happen if we don't have valid GPS data and
   * an overflow has happened, handle within 20 seconds of
   * reaching microdelta */
  if (IN_OVERFLOW && ((microdelta - micros()) > 20e6))
  {
    /* Set new reference using internal clock */
    rf_send_debug ("Roll reference: Manual.");
    referencesecond += TIME_FROM_REFERENCE / 1e6;
    microdelta = micros ();
    IN_OVERFLOW = false;
    HAS_SYNC    = false;
    HAS_SYNC_REFERENCE = false;
  }

  /* Handle incoming GPS telegrams on serial line {{{
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

        gps_parse (); // Complete telegram received
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

  HAS_TIME = gps_data.valid;

  /* Check if we still have synced clock */
  HAS_SYNC = (HAS_SYNC && gps_data.valid && (millis() - lastsync < 1000));
}

# endif

/* vim: set filetype=arduino :  */

