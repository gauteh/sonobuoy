/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Communication protocol over Synapse RF Wireless.
 *
 */

# include <stdio.h>
# include <string.h>
# include "wirish.h"

# include "buoy.h"
# include "rf.h"
# include "ads1282.h"
# include "gps.h"

using namespace std;

namespace Buoy {
  RF::RF () {
    laststatus = 0;
    lastbatch = 0;
    continuous_transfer = false;
    rf = this;
  }

  void RF::setup (BuoyMaster *b) {
    ad = b->ad;
    gps = b->gps;

    RF_Serial.begin (RF_BAUDRATE);
    send_debug ("[RF] RF subsystem initiated.");
  }

  void RF::loop () {
    /* Status should be send every second */
    if (millis () - laststatus > 1000) send_status ();

    /* Loop must run at least 2x speed (Nyquist) of batchfilltime */
    if (continuous_transfer) {
      if (ad->batchready != lastbatch) {
       ad_message (AD_DATA_BATCH);
      }
    }
  }

  void RF::send_status () {
    ad_message (AD_STATUS);
    gps_message (GPS_STATUS);

    laststatus = millis ();
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
        sprintf (buf, "$AD,S,%lu,%lu,0x%08lX,0x%08hX*", ad->position, ad->batchfilltime, ad->value, ad->reg.raw[1]);
        APPEND_CSUM (buf);

        RF_Serial.println (buf);

        break;

      case AD_DATA_BATCH:
        /* Send BATCH_LENGTH samples */

        /* Format and protocol:

         * 1. Initiate binary data stream:

         $AD,D,[k = number of samples],[reference]*CC

         * 2. Send one $ to indicate start of data

         * 3. Send k number of samples: 4 bytes * k

         * 4. Send k number of timestamps: 4 bytes * k

         * 5. Send end of data with checksum

         ** In case reference has been updated in batch, two separate batches
         ** are sent.

         */
        {
          uint32_t start  =  (lastbatch * BATCH_LENGTH);
          lastbatch       =  (lastbatch + 1) % BATCHES;
          uint32_t length = BATCH_LENGTH;
          uint32_t ref    = gps->referencesecond;
          bool go         = true;
          bool update_ref = false;

          /* Reference updated in this batch */
          if (gps->update_reference &&
              gps->update_reference_position > start &&
              gps->update_reference_position < (start + BATCH_LENGTH))
          {
            length  = gps->update_reference_position - start;
            ref     = gps->previous_reference;
            update_ref = true;
          }

          while (go) {
            sprintf (buf, "$AD,D,%lu,%lu*", length, ref);
            APPEND_CSUM (buf);
            RF_Serial.println (buf);

            delayMicroseconds (100);

            byte csum = 0;

            /* Write '$' to signal start of binary data */
            RF_Serial.write ('$');

            //uint32_t lasts = 0;
            uint32_t s;

            for (uint32_t i = 0; i < length; i++)
            {
              s = ad->values[start + i];
              /* MSB first (big endian), means concatenating bytes on RX will
               * result in LSB first; little endian. */
              RF_Serial.write ((byte*)(&s), 4);

              csum = csum ^ ((byte*)&s)[0];
              csum = csum ^ ((byte*)&s)[1];
              csum = csum ^ ((byte*)&s)[2];
              csum = csum ^ ((byte*)&s)[3];

              //lasts = s;

              delayMicroseconds (100);
            }

            /* Send time stamps */
            uint32_t t = 0;
            for (uint32_t i = 0; i < length; i++)
            {
              t = ad->times[start + i];

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

            /*
            SerialUSB.print ("[RF] Last sample: 0x");
            SerialUSB.println (lasts, HEX);
            rf_send_debug_f ("[RF] Last sample: 0x%lX", lasts);
            */

            /* Prepare for last part of batch in case reference has been updated */
            if (update_ref) {
              start   = start + length;
              length  = BATCH_LENGTH - length;
              ref     = gps->referencesecond;
              update_ref = false;
              send_debug ("[RF] Sending last part of batch (updated reference).");
            } else {
              go = false;
            }
          }
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
        sprintf (buf, "$GPS,S,%d,%d,%s,%c,%s,%c,%lu,%lu,%02d%02d%02d,%c,%c,%c,%c*", gps->gps_data.lasttype, gps->gps_data.received, gps->gps_data.latitude, (gps->gps_data.north ? 'N' : 'S'), gps->gps_data.longitude, (gps->gps_data.east ? 'E' : 'W'), (uint32_t) gps->lastsecond, gps->gps_data.time, gps->gps_data.day, gps->gps_data.month, gps->gps_data.year, (gps->gps_data.valid ? 'Y' : 'N'), (gps->HAS_TIME ? 'Y' : 'N'), (gps->HAS_SYNC ? 'Y' : 'N'), (gps->HAS_SYNC_REFERENCE ? 'Y' : 'N'));

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

  void RF::start_continuous_transfer () {
    continuous_transfer = true;
  }

  void RF::stop_continuous_transfer () {
    continuous_transfer = false;
  }
}

/* vim: set filetype=arduino :  */

