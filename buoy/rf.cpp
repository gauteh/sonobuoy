/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Communication protocol over Synapse RF Wireless.
 *
 */

# include <stdio.h>
# include <string.h>
# include "wirish.h"

# include "rf.h"
# include "ads1282.h"
# include "gps.h"

using namespace std;

namespace Buoy {
  RF::RF () {

  }

  void RF::setup () {
    RF_Serial.begin (RF_BAUDRATE);
    send_debug ("[RF] RF subsystem setup.");

  }

  void RF::loop () {
    send_status ();
  }

  void RF::send_status () {
    ad_message (AD_STATUS);
    //gps_message (GPS_STATUS);
  }

  void RF::send_debug (const char * msg)
  {
    /* Format:
     * $DBG,[msg]*CS
     *
     */

    char buf[RF_BUFLEN];
    sprintf(buf, "$DBG,%s*", msg);
    APPEND_CSUM (buf);

    RF_Serial.println (buf);
  }

  void RF::ad_message (RF_AD_MESSAGE messagetype)
  {
    char buf[RF_BUFLEN];

    switch (messagetype)
    {
      case AD_STATUS:
        // $AD,S,[queue position], [queue fill time],[value],[config]*CS
        sprintf (buf, "$AD,S,%lu,%lu,0x%08lX,0x%08hX*", Ad->position, Ad->batchfilltime, Ad->value, Ad->reg.raw[1]);
        APPEND_CSUM (buf);

        RF_Serial.println (buf);

        break;

      case AD_DATA_BATCH:
        /* Send AD_DATA_BATCH_LEN samples */
        # define AD_DATA_BATCH_LEN (QUEUE_LENGTH / 2)

        /* TODO: In case reference is changing within batch, communicate it.. */

        /* Format:

         * 1. Initiate binary data stream:

         $AD,D,[k = number of samples],[reference]*CC

         * 2. Send one $ to indicate start of data

         * 3. Send k number of samples: 4 bytes * k

         * 4. Send k number of timestamps: 4 bytes * k

         * 5. Send end of data with checksum

         */
        {
          int start = (Ad->batchready == 1 ? 0 : AD_DATA_BATCH_LEN);

          sprintf (buf, "$AD,D,%d,%lu*", AD_DATA_BATCH_LEN, Gps->referencesecond);
          APPEND_CSUM (buf);
          RF_Serial.println (buf);

          delayMicroseconds (100);

          byte csum = 0;

          /* Write '$' to signal start of binary data */
          RF_Serial.write ('$');

          //uint32_t lasts;
          uint32_t *s;

          for (int i = 0; i < AD_DATA_BATCH_LEN; i++)
          {
            s = (uint32_t*) &(Ad->values[start + i]);
            /* MSB first (big endian), means concatenating bytes on RX will
             * result in LSB first; little endian. */
            RF_Serial.write (s, 4);

            csum = csum ^ ((*s >> 24) &0xff);
            csum = csum ^ ((*s >> 16) &0xff);
            csum = csum ^ ((*s >>  8) &0xff);
            csum = csum ^ ((*s      ) &0xff);

            //lasts = *s;

            delayMicroseconds (100);
          }

          /* Send time stamps */
          uint32_t t = 0;
          for (int i = 0; i < AD_DATA_BATCH_LEN; i++)
          {
            t = Ad->times[start + i];

            /* Writes MSB first */
            RF_Serial.write ((byte*)(&t), 4);

            csum = csum ^ ((byte*)&t)[0];
            csum = csum ^ ((byte*)&t)[1];
            csum = csum ^ ((byte*)&t)[2];
            csum = csum ^ ((byte*)&t)[3];
          }

          /* Send end of data with Checksum */
          sprintf (buf, "$AD,DE," F_CSUM "*", csum);
          APPEND_CSUM (buf);
          RF_Serial.println (buf);
          delayMicroseconds (100);
        }
        break;

      default:
        return;
    }
  }

  void RF::gps_message (RF_GPS_MESSAGE messagetype)
  {
    char buf[RF_BUFLEN];

    switch (messagetype)
    {
      case GPS_STATUS:
        // $GPS,S,[lasttype],[telegrams received],[lasttelegram],Lat,Lon,unixtime,time,date,Valid,HAS_TIME,HAS_SYNC,HAS_SYNC_REFERENCE*CS
        // Valid: Y = Yes, N = No
        sprintf (buf, "$GPS,S,%d,%d,%s,%c,%s,%c,%lu,%lu,%02d%02d%02d,%c,%c,%c,%c*", Gps->gps_data.lasttype, Gps->gps_data.received, Gps->gps_data.latitude, (Gps->gps_data.north ? 'N' : 'S'), Gps->gps_data.longitude, (Gps->gps_data.east ? 'E' : 'W'), Gps->lastsecond, Gps->gps_data.time, Gps->gps_data.day, Gps->gps_data.month, Gps->gps_data.year, (Gps->gps_data.valid ? 'Y' : 'N'), (Gps->HAS_TIME ? 'Y' : 'N'), (Gps->HAS_SYNC ? 'Y' : 'N'), (Gps->HAS_SYNC_REFERENCE ? 'Y' : 'N'));

        break;

      default:
        return;
    }

    APPEND_CSUM (buf);
    RF_Serial.println (buf);
  }

  byte RF::gen_checksum (char *buf)
  {
  /* Generate checksum for NULL terminated string
   * (skipping first and last char) */

    byte csum = 0;
    int len = strlen(buf);

    for (int i = 1; i < (len-1); i++)
      csum = csum ^ ((byte)buf[i]);

    return csum;
  }

  bool RF::test_checksum (char *buf)
  {
    /* Input: String including $ and * with HEX decimal checksum
     *        to test. NULL terminated.
     */
    int len = strlen(buf);

    uint16_t csum = 0;
    sscanf (&(buf[len-2]), F_CSUM, &csum);

    uint32_t tsum = 0;
    for (int i = 1; i < (len - 3); i++)
      tsum = tsum ^ (uint8_t)buf[i];

    return tsum == csum;
  }

}

/* vim: set filetype=arduino :  */

