/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-05
 *
 * Interface to standard NMEA GPS.
 *
 *
 */

# ifndef GPS
# define GPS

# include "buoy.h"
# include "rf.h"
# include "gps.h"

char gps_buf[TELEGRAM_LEN + 2];
int  gps_buf_pos   = 0;

/* Structure holding the latest GPS data */
GPS_DATA gps_data;

void gps_setup ()
{
  GPS_Serial.begin (GPS_BAUDRATE);

  gps_buf[0] = 0;
  gps_buf_pos = 0;

  gps_data.received = 0;
  gps_data.valid = false;
  gps_data.latitude[0] = 0;
  gps_data.north = true;
  gps_data.longitude[0] = 0;
  gps_data.time = 0;
  gps_data.lasttype = UNSPECIFIED;
}


bool HAS_LEAP_SECONDS = false;
bool HAS_TIME         = false;
bool HAS_SYNC         = false;
bool IN_OVERFLOW      = false;

volatile ulong referencesecond = 0;
volatile ulong lastsecond      = 0;
volatile ulong microdelta      = 0;

volatile ulong lastmicros      = 0;

void gps_sync_pulse ()
{
  /* Synchronize time stamp clock */
  HAS_SYNC = true;

  /* Last second should be received every second */
  if (gps_data.valid) {
    lastsecond++;

    microdelta = micros () - (1e6 * (lastsecond - referencesecond));

    /* Is reset because of new microdelta calculation */
    IN_OVERFLOW = false;
  }

  /* For overrun handling */
  lastmicros = micros ();
}

void gps_roll_reference ()
{
  /* Change referencesecond to latest */
  if (gps_data.valid) {
    microdelta = microdelta - (1e6 * (lastsecond - referencesecond));
    referencesecond = lastsecond;
  }
}

void gps_update_second ()
{
  /* Got a telegram with UTC time information */
  HAS_TIME = gps_data.valid;
  HAS_SYNC = (HAS_SYNC && gps_data.valid);

  if (gps_data.valid) {
    /* Create time in utc seconds from GPS data */

  }
}

void gps_parse ()
{
  /* GPS parser {{{
   *
   * Telegram types:
   *
   * GPRMC
   * GPGGA
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

    /* The last token, where parsing has reached the full length
     * of gps_buf, will contain the checksum.
     *
     * We don't need this since we have already checked it beforehand.
     *
     */

    if (i < len) {
      if (tokeni == 0) {
        /* Determine telegram type */
        if (strcmp(token, "$GPRMC") == 0)
          type = GPRMC;
        else if (strcmp(token, "$GPGGA") == 0)
          type = GPGGA;
        else if (strcmp(token, "$GPGSA") == 0)
          type = GPGSA;
        else {
          /* Cancel parsing */
          type = UNKNOWN;
          return;
        }
        gps_data.lasttype = type;
      } else {
        switch (type)
        {
          case GPRMC:
            switch (tokeni)
            {
              case 1:
                sscanf (token, "%lu.%lu", &(gps_data.time), &(remainder));
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
                sscanf (token, "%d", &(gps_data.date));
                break;

              case 10: /* Magnetic declination not supported by device */
                gps_update_second ();

              default:
                break;
            }
            break;
          case GPGGA:
            switch (tokeni)
            {

            }
            break;

          case GPGSA:
            switch (tokeni)
            {

            }
            break;

          default:
            /* Having reached here on an unknown or unspecified telegram
             * parsing is cancelled. */
            return;
        }
      }
    }
    tokeni++;
  }
  /* Done parser }}} */
}

void gps_loop ()
{

  /* Check wether to update reference second */
  if ((lastsecond - referencesecond) > ROLL_REFERENCE)
  {
    gps_roll_reference ();
  }

  /* This can happen if we don't have valid GPS data and
   * an overflow has happened, handle within 10 seconds of
   * reaching microdelta */
  if (IN_OVERFLOW && ((microdelta - micros()) > (ulong)10e6))
  {
    /* Set new reference using internal clock */
    referencesecond += TIME_FROM_REFERENCE / (ulong)10e6;
    microdelta = micros ();
    IN_OVERFLOW = false;
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
}

# endif

/* vim: set filetype=arduino :  */

