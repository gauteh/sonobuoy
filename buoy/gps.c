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

# define GPS_BUF_LEN  1024
# define TELEGRAM_LEN 80

# define GPS_BAUDRATE 4800
# define GPS_Serial Serial1

char gps_buf[TELEGRAM_LEN + 2];
int  gps_buf_pos   = 0;

char gps_rmc[TELEGRAM_LEN];


void gps_setup ()
{
  Serial.println ("[GPS] Setting up GPS serial interface (GPS_Serial)..");
  GPS_Serial.begin (GPS_BAUDRATE);

  gps_buf[0] = 0;
  gps_rmc[0] = 0;
}

void gps_loop ()
{
  int ca = GPS_Serial.available (); 

  while (ca > 0) {
    int c = GPS_Serial.read ();

    if (c == '\r' || c == '\n')
      continue;

    if (((char)c) == '$') {
      gps_buf[gps_buf_pos] = 0;

      for (int i = 0; i < gps_buf_pos; i++)
      {
        strcpy (gps_rmc, gps_buf);
        gps_rmc[gps_buf_pos] = 0;
      }
      gps_buf_pos = 0;
    }

    gps_buf[gps_buf_pos] = (char)c;
    gps_buf_pos++;
    ca--;
  }
}

void gps_status (HardwareSerial s)
{
  s.print ("[GPS] ");
  int i = 1;
  do {
    s.print (gps_rmc[i]);
    i++;
  } while (gps_rmc[i] != 0 && i < TELEGRAM_LEN);

  s.println ("$");
}

# endif

/* vim: set filetype=arduino :  */

