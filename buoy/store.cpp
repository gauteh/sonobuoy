/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-02-05
 *
 * SD store
 *
 */

# include "wirish.h"
# include "HardwareSPI.h"
# include "store.h"
# include "buoy.h"
# include "rf.h"
# include "ads1282.h"

# include <stdio.h>

namespace Buoy {
  Store::Store () {

  }

  void Store::setup (BuoyMaster *b) {
    rf = b->rf;
    ad = b->ad;
    gps = b->gps;

    pinMode (SD_CS, OUTPUT);
    pinMode (SD_MOSI, OUTPUT);
    pinMode (SD_MISO, INPUT);
    pinMode (SD_SCLK, OUTPUT);

    spi = new HardwareSPI(1);
    spi.begin (SPI_18MHZ, MSBFIRST, 0);

    lastsd = millis ();
    init ();
  }

  void Store::init ()
  {
    SD_AVAILABLE = (sd.init (, SD_CS) & (sd.card()->cardSize() > 0));
    SD_AVAILABLE &= (sd.card()->errorCode () == 0);

    if (SD_AVAILABLE)
    {
      rf->send_debug ("SD card ready.");

      sd.chdir ("/", true);
      open_index ();
      open_data ();

    } else {
      rf->send_debug ("Could not init SD.");
      current_index.id = 0;
    }
  }

  void Store::open_index ()
  {
    uint32_t n = 0;

    rf->send_debug ("Opening index..");

    uint32_t i;
    if (sd.exists("LASTID.LON")) {
      SdFile fl ("LASTID.LON", O_READ);

      n = fl.read (reinterpret_cast<char*>(&i), sizeof(uint32_t));

      if (n < sizeof(i)) i = 1;

      fl.close ();
    } else {
      i = 1;
    }

    rf_send_debug_f ("Last id: %lu..", i);
    next_index (i);
  }

  /* Open next index file */
  void Store::next_index (uint32_t i)
  {
    // i is LASTID

    // TODO: Check if we have reached MAXID

    if (i > 100) i = 1;  // DEBUG

    /* Walk through subsequent indexes above lastid and take next free */
    int n = 0;
    char buf[8+5];
    bool newi = false;

# if DIRECT_SERIAL
    SerialUSB.print ("Checking for subseq. indexes from: ");
    SerialUSB.println (i);
# endif

    while (!newi)
    {
      sprintf (buf, "%lu.IND", i);
# if DIRECT_SERIAL
      SerialUSB.println(buf);
# endif

      if (!sd.exists(buf)) {
        newi = true; /* Found new index file at id I */

# if DIRECT_SERIAL
        SerialUSB.print ("Found free index at: ");
        SerialUSB.println (i);
# endif
        rf_send_debug_f ("Next index: %lu", i);

      }
      i++;
    }
    i--;

    // Open new index
    current_index.version = STORE_VERSION;
    current_index.id = i;
    current_index.sample_l = SAMPLE_LENGTH;
    current_index.timestamp_l = TIMESTAMP_LENGTH;
    current_index.samples = 0;
    current_index.nrefs = 0;

    write_index ();
  }

  void Store::write_index ()
  {
    char buf[8+5];
    rf_send_debug_f ("Writing index: %lu..", current_index.id);

    if (current_index.id != 0) {
      sprintf (buf, "%lu.IND", current_index.id);

      SdFile fi (buf, O_CREAT | O_WRITE | O_TRUNC);
      fi.write (reinterpret_cast<char*>(&current_index.version), sizeof(current_index.version));
      fi.write (reinterpret_cast<char*>(&current_index.id), sizeof(current_index.id));
      fi.write (reinterpret_cast<char*>(&current_index.sample_l), sizeof(current_index.sample_l));
      fi.write (reinterpret_cast<char*>(&current_index.timestamp_l), sizeof(current_index.timestamp_l));
      fi.write (reinterpret_cast<char*>(&current_index.samples), sizeof(current_index.samples));
      fi.write (reinterpret_cast<char*>(&current_index.nrefs), sizeof(current_index.nrefs));
      fi.write (reinterpret_cast<char*>(&current_index.refs), current_index.nrefs * sizeof(uint32_t));
      fi.sync ();
      fi.close ();

      /* Write back last index */
      SdFile fl("LASTID.LON", O_CREAT | O_WRITE | O_TRUNC);
      fl.write (reinterpret_cast<char*>(&(current_index.id)), sizeof(current_index.id));
      fl.sync ();
      fl.close ();
    }

# if DIRECT_SERIAL
    sd.ls ();
# endif

    SD_AVAILABLE &= (sd.card()->errorCode () == 0);
  }

  /* Open new index and data file */
  void Store::roll_data_file ()
  {
    rf->send_debug ("Syncing index and data and rolling..");

    /* Truncate data file to actual size */
    //sd_data.truncate (sd_data.curPosition ());

    sd_data.sync ();
    sd_data.close ();

    /* Open new index */
    write_index ();
    next_index (current_index.id);

    /* Open new data file */
    open_data ();
  }

  bool referencewritten = false;

  /* Write new batch of samples */
  void Store::write_batch ()
  {
    if (!SD_AVAILABLE) {
      rf_send_debug_f ("No write: error: %02X.", sd.card ()->errorCode ());
      return;
    }

    /* Check if we have room for samples in store */
    if (current_index.samples > (MAX_SAMPLES_PER_FILE - BATCH_LENGTH))
    {
      roll_data_file ();
    }


    /* Check if we have exceeeded this store */
    if (sd_data.curPosition () > (SD_DATA_FILE_SIZE - (BATCH_LENGTH * (SAMPLE_LENGTH + TIMESTAMP_LENGTH))))
    {
      roll_data_file ();
    }

    /* In case we are in update_reference, check if we have space for
     * one more reference */
    if (gps->update_reference) {
      if (sd_data.curPosition () > (SD_DATA_FILE_SIZE - (BATCH_LENGTH * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)) - SD_REFERENCE_LENGTH))
      {
        roll_data_file ();
      }
    }

    /* Write first reference since this is a new file */
    if (!referencewritten) {
      write_reference (gps->referencesecond);
    }

    /* Writing entries */
    rf_send_debug_f ("Writing entries to data file from sample: %lu", current_index.samples);
    uint32_t s = (ad->batchready == 1 ? 0 : (BATCH_LENGTH));
    for (uint32_t i = s; i <  s + (BATCH_LENGTH); i++)
    {
      /* Write reference at correct position */
      if (gps->update_reference && i == gps->update_reference_position) {
        rf_send_debug_f ("In-loop reference queue: %lu", gps->update_reference_position);
        write_reference (gps->referencesecond);
      }

      sd_data.write (reinterpret_cast<char*>((uint32_t*) &(ad->times[i])), sizeof(uint32_t));
      sd_data.write (reinterpret_cast<char*>((uint32_t*)  &(ad->values[i])), sizeof(uint32_t));
    }

    current_index.samples += BATCH_LENGTH;

    sd_data.sync ();
    SD_AVAILABLE &= (sd.card()->errorCode () == 0);
  }

  /* Open data file */
  void Store::open_data ()
  {
    char fname[13];
    sprintf (fname, "%lu.DAT", current_index.id);

    SD_AVAILABLE = sd_data.open (fname, O_CREAT | O_WRITE | O_TRUNC);
    SD_AVAILABLE &= (sd.card()->errorCode () == 0);

    referencewritten = false;
  }

  void Store::write_reference (uint32_t ref)
  {
    rf_send_debug_f ("Write reference: %lu", ref);
    if (SD_AVAILABLE)
    {
      /* Check if we have exceeded MAX_REFERENCES */
      if (current_index.nrefs >= (MAX_REFERENCES - 1)) {
        roll_data_file ();
      }

      /* Check if there is more space in data file */
      if (sd_data.curPosition () > (SD_DATA_FILE_SIZE - SD_REFERENCE_LENGTH)) {
        roll_data_file ();
      }

      /* Update index */
      current_index.refs[current_index.nrefs] = sd_data.curPosition ();

      /* Pad with 0 */
      for (uint32_t i = 0; i < (3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); i++)
        sd_data.write ((byte)0);

      sd_data.write (reinterpret_cast<char*>(&(current_index.nrefs)), sizeof(uint32_t));
      sd_data.write (reinterpret_cast<char*>(&(ref)), sizeof(uint32_t));
      sd_data.write (reinterpret_cast<char*>(&(sd_status)), sizeof(uint32_t));

      /* Pad with 0 */
      for (uint32_t i = 0; i < (3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); i++)
        sd_data.write ((byte)0);

      current_index.nrefs++;
      referencewritten = true;

      SD_AVAILABLE &= (sd.card()->errorCode () == 0);
    }
  }

  void Store::loop ()
  {
    /* Try to set up SD card, 5 sec delay  */
    if (!SD_AVAILABLE && (millis () - lastsd) > 5000) {
      rf_send_debug_f ("SD error code: %02X.", sd.card ()->errorCode ());
      init ();
      lastsd = millis ();
    }
  }
}

/* vim: set filetype=arduino :  */

