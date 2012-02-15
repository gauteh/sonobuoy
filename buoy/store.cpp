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
# include "gps.h"

# include <stdio.h>

namespace Buoy {
  Store::Store () {
    logf_id = 0;
  }

  void Store::setup (BuoyMaster *b) {
    rf = b->rf;
    ad = b->ad;
    gps = b->gps;

    /*
    pinMode (SD_CS, OUTPUT);
    pinMode (SD_MOSI, OUTPUT);
    pinMode (SD_MISO, INPUT);
    pinMode (SD_SCLK, OUTPUT);
    */

    spi = new HardwareSPI(SD_SPI);

    lastsd = millis ();
    lastbatch = ad->batchready;
    continuous_write = false;
    init ();

    rf->send_debug ("[SD] Store subsystem initiated.");
  }

  void Store::init ()
  {
    rf->send_debug ("[SD] Init SD card.");
# if DIRECT_SERIAL
    SerialUSB.println ("[SD] Init SD card.");
# endif

    /* Initialize SD card */
    spi->begin (SPI_281_250KHZ, MSBFIRST, 0);
    SD_AVAILABLE = (card.init (spi, SD_CS) & (card.cardSize() > 0));
    SD_AVAILABLE &= (card.errorCode () == 0);

    /* Beef up SPI after init is finished */
    spi->begin (SPI_4_5MHZ, MSBFIRST, 0);

    /* Initialize FAT volume */
    SD_AVAILABLE &= volume.init (&card, 1);

    /* Open root directory */
    SD_AVAILABLE &= root.openRoot (&volume);

    if (SD_AVAILABLE)
    {
      rf->send_debug ("[SD] Card ready.");

      open_index ();
      open_data ();

      open_next_log ();
    } else {
      rf->send_debug ("[SD] [Error] Could not init SD.");
      current_index.id = 0;
    }
  }

  void Store::open_index ()
  {
    uint32_t n = 0;

    rf->send_debug ("[SD] Opening index..");

    uint32_t i;
    SdFile fl;

    if (fl.open (&root, "LASTID.LONG", O_READ)) {
      n = fl.read (reinterpret_cast<char*>(&i), sizeof(uint32_t));

      if (n < sizeof(i)) i = 1;

      fl.close ();
    } else {
      i = 1;
    }

    rf_send_debug_f ("[SD] Last id: %lu..", i);
    next_index (i);
  }

  /* Open next available log file */
  void Store::open_next_log ()
  {
    char buf[8+5];
    sprintf (buf, "%lu.LOG", logf_id);

    while (logf.open (&root, buf, O_READ)) {
      logf.close ();
      logf_id++;
      sprintf (buf, "%lu.LOG", logf_id);
    }
    logf.close ();

    logf.open (&root, buf, O_READ | O_CREAT | O_WRITE);
    rf_send_debug_f ("[SD] Opened log: %lu.LOG", logf_id);
  }

  void Store::log (const char *s)
  {
    if (SD_AVAILABLE) {
      /* Open next log file */
      if (logf.curPosition () > MAX_LOG_SIZE) {
        logf.close ();
        open_next_log ();
      }

      if (!SD_AVAILABLE) return;

      char buf[13];
      sprintf (buf, "[%lu]", gps->lastsecond);
      logf.write (buf);
      logf.write (s);
      logf.write ('\n');
    }
  }

  /* Open next index file */
  void Store::next_index (uint32_t i)
  {
    // i is LASTID

    // TODO: Check if we have reached MAXID

    if (i > 100) i = 1;  // DEBUG

    /* Walk through subsequent indexes above lastid and take next free */
    char buf[8+5];
    bool newi = false;

# if DIRECT_SERIAL
    SerialUSB.print ("[SD] Checking for subseq. indexes from: ");
    SerialUSB.println (i);
# endif

    SdFile fi;

    while (!newi)
    {
      sprintf (buf, "%lu.IND", i);
# if DIRECT_SERIAL
      SerialUSB.println(buf);
# endif

      if (!fi.open(&root, buf, O_READ)) {
        newi = true; /* Found new index file at id I */

# if DIRECT_SERIAL
        SerialUSB.print ("[SD] Found free index at: ");
        SerialUSB.println (i);
# endif
        rf_send_debug_f ("[SD] Next index: %lu", i);

      } else {
        fi.close ();
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
    rf_send_debug_f ("[SD] Writing index: %lu..", current_index.id);

    if (current_index.id != 0) {
      sprintf (buf, "%lu.IND", current_index.id);

      SdFile fi;
      fi.open (&root, buf, O_CREAT | O_WRITE | O_TRUNC);
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
      SdFile fl;
      fl.open (&root, "LASTID.LON", O_CREAT | O_WRITE | O_TRUNC);
      fl.write (reinterpret_cast<char*>(&(current_index.id)), sizeof(current_index.id));
      fl.sync ();
      fl.close ();
    }

    SD_AVAILABLE &= (card.errorCode () == 0);
  }

  /* Open new index and data file */
  void Store::roll_data_file ()
  {
    rf->send_debug ("[SD] Syncing index and data and rolling..");

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
      rf_send_debug_f ("[SD] No write: error: %02X.", card.errorCode ());
      return;
    }

    uint32_t s =  lastbatch * BATCH_LENGTH;
    lastbatch  =  (lastbatch + 1) % BATCHES;

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

    // TODO: Is this done in write_reference () ?
    /* In case we are in update_reference, check if we have space for
     * one more reference */
    if (gps->update_reference && gps->update_reference_position > s && gps->update_reference_position < (s + BATCH_LENGTH)) {
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
    rf_send_debug_f ("[SD] Writing entries to data file from sample: %lu", current_index.samples);


    for (uint32_t i = s; i <  s + (BATCH_LENGTH); i++)
    {
      /* Write reference at correct position */
      if (gps->update_reference && i == gps->update_reference_position) {
        rf_send_debug_f ("[SD] In-loop reference queue: %lu", gps->update_reference_position);
        write_reference (gps->referencesecond);
      }

      sd_data.write (reinterpret_cast<char*>((uint32_t*) &(ad->times[i])), sizeof(uint32_t));
      sd_data.write (reinterpret_cast<char*>((uint32_t*)  &(ad->values[i])), sizeof(uint32_t));
    }

    current_index.samples += BATCH_LENGTH;

    sd_data.sync ();
    SD_AVAILABLE &= (card.errorCode () == 0);
  }

  /* Open data file */
  void Store::open_data ()
  {
    char fname[13];
    sprintf (fname, "%lu.DAT", current_index.id);

    SD_AVAILABLE = sd_data.open (&root, fname, O_CREAT | O_WRITE | O_TRUNC);
    SD_AVAILABLE &= (card.errorCode () == 0);

    referencewritten = false;
  }

  void Store::write_reference (uint32_t ref)
  {
    rf_send_debug_f ("[SD] Write reference: %lu", ref);
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
      for (uint32_t i = 0; i < (SD_REFERENCE_PADN * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); i++)
        sd_data.write ((byte)0);

      sd_data.write (reinterpret_cast<char*>(&(current_index.nrefs)), sizeof(uint32_t));
      sd_data.write (reinterpret_cast<char*>(&(ref)), sizeof(uint32_t));
      sd_data.write (reinterpret_cast<char*>(&(sd_status)), sizeof(uint32_t));

      /* Pad with 0 */
      for (uint32_t i = 0; i < (SD_REFERENCE_PADN * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); i++)
        sd_data.write ((byte)0);

      current_index.nrefs++;
      referencewritten = true;

      SD_AVAILABLE &= (card.errorCode () == 0);
    }
  }

  void Store::loop ()
  {
    /* Try to set up SD card, 5 sec delay  */
    if (!SD_AVAILABLE && (millis () - lastsd) > 5000) {
      rf_send_debug_f ("[SD] SD error code: %02X.", card.errorCode ());
      init ();
      lastsd = millis ();
    }

    /* Check if new batch is ready */
    /* Loop must run at least 2x speed (Nyquist) of batchfilltime */
    if (SD_AVAILABLE && continuous_write) {
      if (ad->batchready != lastbatch) {
        write_batch ();
      }
    }
  }

  void Store::start_continuous_write () {
    continuous_write = true;
  }

  void Store::stop_continuous_write () {
    continuous_write = false;
  }
}

/* vim: set filetype=arduino :  */

