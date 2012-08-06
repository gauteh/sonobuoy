/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-31
 *
 * Communication protocol over Synapse RF Wireless.
 *
 */


# include <stdlib.h>
# include <string.h>

# include "buoy.h"

# include "wirish.h"

# include "rf.h"
# include "ads1282.h"
# include "gps.h"
# include "store.h"

using namespace std;

namespace Buoy {
  RF::RF () {
    rf = this;

    rf_buf_pos = 0;
  }

  void RF::setup (BuoyMaster *b) {
    ad    = b->ad;
    gps   = b->gps;
    store = b->store;

    RF_Serial.begin (RF_BAUDRATE);
  }

  void RF::loop () {
    /* Handle incoming RF telegrams on serial line (non-blocking) {{{
     *
     *
     * States:
     * 0 = Waiting for start of telegram $
     * 1 = Receiving (between $ and *)
     * 2 = Waiting for Checksum digit 1
     * 3 = Waiting for Checksum digit 2
     */
    static int state = 0;

    while (RF_Serial.available () > 0) {
      char c = (char)RF_Serial.read ();
      //SerialUSB.println (c);

      if (rf_buf_pos >= RF_SERIAL_BUFLEN) {
        state       = 0;
        rf_buf_pos  = 0;
        return;
      }

      switch (state)
      {
        case 0:
          if (c == '$') {
            rf_buf[0]   = '$';
            rf_buf_pos  = 1;
            state       = 1;
          }
          break;

        case 2:
          state = 3;
        case 1:
          if (c == '*')
            state = 2;

          rf_buf[rf_buf_pos] = c;
          rf_buf_pos++;
          break;

        case 3:
          rf_buf[rf_buf_pos] = c;
          rf_buf_pos++;
          rf_buf[rf_buf_pos] = 0;

          parse (); // Complete telegram received
          rf_buf_pos  = 0;
          state       = 0;
          return;
          break;

        /* Should not be reached. */
        default:
          state       = 0;
          rf_buf_pos  = 0;
          return;
          break;
      }
    }

    /* }}} Done telegram handler */
  }

  void RF::parse ()
  {
    /* RF parser (non-blocking) {{{
     *
     *
     */
    //SerialUSB.print ("Parsing..:");
    //SerialUSB.println (rf_buf);

    RF_TELEGRAM type = UNSPECIFIED;
    int tokeni = 0;
    int len    = rf_buf_pos - 1; // Excluding NULL terminator
    int i      = 0;

    /* Test checksum before parsing */
    if (!test_checksum (rf_buf)) {
      send_error (E_BADCOMMAND);
      return;
    }

    /* Parse */
    while (i < len)
    {
      //SerialUSB.println ("nc");
      /*
      uint32_t ltmp = 0;
      uint32_t remainder = 0;
      */

      char token[80]; // Max length of token
      int j = 0;
      /* Get next token */
      while ((rf_buf[i] != ',' && rf_buf[i] != '*') && i < len) {
        //SerialUSB.println ("nt");
        token[j] = rf_buf[i];

        i++;
        j++;
      }
      i++; /* Skip delimiter */

      token[j] = 0;
# if DEBUG_VERB
      SerialUSB.print ("T ");
      SerialUSB.print (tokeni);
      SerialUSB.println (token);
# endif

      if (i < len) {
        if (tokeni == 0) {
          /* Determine telegram type */
          if (strcmp(token, "$GS") == 0) {
            simple_parser (GETSTATUS);
            return;
          }
          else if (strcmp(token, "$GLID") == 0) {
            simple_parser (GETLASTID);
            return;
          }
          else if (strcmp(token, "$GIDS") == 0)
            type = GETIDS;
          else if (strcmp(token, "$GID") == 0)
            type = GETID;
          else if (strcmp(token, "$GB") == 0)
            type = GETBATCH;
          else {
            /* Cancel parsing */
# if DEBUG_VERB
            SerialUSB.print ("Unknown command: ");
            SerialUSB.println (token);
# endif
            type = UNKNOWN;
            send_error (E_UNKNOWNCOMMAND);
            return;
          }
        } else {
          switch (type)
          {
            // GETIDS {{{
            case GETIDS:
              switch (tokeni)
              {
                /* first token specifies starting id to send */
                case 1:
                  {
                  id = atoi (token);
                  if (id < 1) {
                    send_error (E_BADCOMMAND);
                    return;
                  }

                  store->send_indexes (id, GET_IDS_N);
                  }
                  break;
              }
              break;
              // }}}

            // GETID {{{
            case GETID:
              switch (tokeni)
              {
                /* first token specifies starting id to send */
                case 1:
                  {
                  id = atoi (token);
                  if (id < 1) {
                    send_error (E_BADCOMMAND);
                    return;
                  }

                  store->send_index (id);
                  }
                  break;
              }
              break;
              // }}}

            // GETBATCH {{{
            case GETBATCH:
              switch (tokeni)
              {
                case 1:
                  {
                  id = atoi (token);
                  if (id < 1 || id > MAX_SANE_ID) {
                    send_error (E_BADCOMMAND);
                    return;
                  }
                  }
                  break;

                case 2:
                  {
                  ref = atoi (token);
                  if (ref > MAX_SANE_REF) {
                    send_error (E_BADCOMMAND);
                    return;
                  }
                  }
                  break;

                case 3:
                  {
                  sample = atoi (token);
                  if (sample > MAX_SANE_SAMPLE) {
                    send_error (E_BADCOMMAND);
                    return;
                  }
                  }
                  break;

                case 4:
                  {
                  length = atoi (token);
                  if (length > MAX_SANE_LENGTH) {
                    send_error (E_BADCOMMAND);
                    return;
                  }

                  store->send_batches (id, ref, sample, length);
                  }
                  break;
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
    return;
    /* Done parser }}} */
  }

  void RF::simple_parser (RF_TELEGRAM type) {
    /* Single token commands  {{{ */
    //SerialUSB.println ("[RF] simplep");
    switch (type) {
      // GETSTATUS {{{
      case GETSTATUS:
        // $GPS,S,[lasttype],[telegrams received],[lasttelegram],Lat,Lon,unixtime,time,date,Valid,HAS_TIME,HAS_SYNC,HAS_SYNC_REFERENCE*CS
        // Valid: Y = Yes, N = No
        //SerialUSB.println ("[RF] Sending status..");
        RF_Serial.print ("$GPS,S,");
        RF_Serial.print (gps->lasttype);
        RF_Serial.print (",");
        RF_Serial.print (gps->received);
        RF_Serial.print (",");
        RF_Serial.print (gps->latitude);
        RF_Serial.print (",");
        RF_Serial.print ((gps->north ? 'N' : 'S'));
        RF_Serial.print (",");
        RF_Serial.print (gps->longitude);
        RF_Serial.print (",");
        RF_Serial.print ((gps->east ? 'E' : 'W'));
        RF_Serial.print (",");
        RF_Serial.print ((uint32_t) gps->lastsecond);
        RF_Serial.print (",");
        RF_Serial.print (gps->time);
        RF_Serial.print (",");
        RF_Serial.print (gps->day);
        RF_Serial.print (gps->month);
        RF_Serial.print (gps->year);
        RF_Serial.print (",");
        RF_Serial.print ((gps->valid ? 'Y' : 'N'));
        RF_Serial.print (",");
        RF_Serial.print ((gps->HAS_TIME ? 'Y' : 'N'));
        RF_Serial.print (",");
        RF_Serial.print ((gps->HAS_SYNC ? 'Y' : 'N'));
        RF_Serial.print (",");
        RF_Serial.print ((gps->HAS_SYNC_REFERENCE ? 'Y' : 'N'));
        RF_Serial.println ("*NN");

        /*
        sprintf (buf, "$GPS,S,%d,%d,%s,%c,%s,%c,%lu,%lu,%02d%02d%02d,%c,%c,%c,%c*", gps->gps_data.lasttype, gps->gps_data.received, gps->gps_data.latitude, (gps->gps_data.north ? 'N' : 'S'), gps->gps_data.longitude, (gps->gps_data.east ? 'E' : 'W'), (uint32_t) gps->lastsecond, gps->gps_data.time, gps->gps_data.day, gps->gps_data.month, gps->gps_data.year, (gps->gps_data.valid ? 'Y' : 'N'), (gps->HAS_TIME ? 'Y' : 'N'), (gps->HAS_SYNC ? 'Y' : 'N'), (gps->HAS_SYNC_REFERENCE ? 'Y' : 'N'));
        APPEND_CSUM (buf);
        RF_Serial.println (buf);
        */

        // $AD,S,[queue position], [queue fill time],[value],[config]*CS
        RF_Serial.print ("$AD,S,");
        RF_Serial.print (ad->position);
        RF_Serial.print (",");
        RF_Serial.print (ad->batchfilltime);
        RF_Serial.print (",");
        RF_Serial.print (ad->value);
        RF_Serial.print (",");
        RF_Serial.print (ad->reg.raw[1]);
        RF_Serial.println ("*NN");

        /*
        sprintf (buf, "$AD,S,%lu,%lu,0x%08lX,0x%08hX*", ad->position, ad->batchfilltime, ad->value, ad->reg.raw[1]);
        APPEND_CSUM (buf);
        RF_Serial.println (buf);
        */
        break;
      // }}}

      // GETLASTID {{{
      case GETLASTID:
        store->send_lastid ();
        break;
        // }}}

      default: break;
    }
    return;
    // }}}
  }

  /* Debug and error messages {{{ */
  void RF::send_error (RF_ERROR code) {
    RF_Serial.print ("$ERR," STRINGIFY(BUOY_ID) ",");
    RF_Serial.print (code, DEC);
    RF_Serial.println ("*NN");

# if DEBUG_WARN
    SerialUSB.print ("[RF] E: ");
    SerialUSB.println (code);
# endif
    /*
    sprintf (buf, "$ERR,%d*", code);
    APPEND_CSUM (buf);
    RF_Serial.println (buf);
    */
  }

  void RF::send_debug (const char * msg)
  {
    /* Format:
     * $DBG,[msg]*CS
     *
     */

    /*
    byte cs = gen_checksum ("DBG,", false);
    cs ^= gen_checksum (msg, false);
    */

    RF_Serial.print   ("$DBG,");
    RF_Serial.print   (msg);
    RF_Serial.print   ("*NN");
    /*
    RF_Serial.print   (cs>>4, HEX);
    RF_Serial.println (cs&0xf, HEX);
    */
  } // }}}
}

/* vim: set filetype=arduino :  */

