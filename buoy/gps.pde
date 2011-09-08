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

# define TELEGRAM_LEN 80

# define GPS_BAUDRATE 4800
# define GPS_Serial Serial1

typedef enum _GPS_TELEGRAM {
  UNSPECIFIED = 0,
  UNKNOWN,
  GPRMC,
  GPGGA,
  GPGSA,

} GPS_TELEGRAM;

typedef struct _GPS_DATA {
  GPS_TELEGRAM lasttype;
  char    lasttelegram[TELEGRAM_LEN];
  int     received; /* Received telegrams */
  bool    valid;
  char    latitude[12];
  bool    north;    /* true = Latitude is north aligned, false = south */
  char    longitude[12];
  bool    east;     /* true = Longitude is east aligned, false = south */
  ulong   time;
  char    speedoverground[6];
  char    courseoverground[6]; /* True north */
  int     date;
} GPS_DATA;

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


void gps_parse ()
{
  /* Telegram types:
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
}

void gps_loop ()
{
/* States:
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
}

# endif

/* vim: set filetype=arduino :  */

