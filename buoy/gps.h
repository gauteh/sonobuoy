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

extern GPS_DATA gps_data;

void gps_setup ();
void gps_loop ();

# endif
