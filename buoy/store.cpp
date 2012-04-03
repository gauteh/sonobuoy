/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-02-05
 *
 * SD store
 *
 */

# include <stdio.h>
# include "wirish.h"
# include "HardwareSPI.h"

# include "store.h"
# include "buoy.h"
# include "rf.h"
# include "ads1282.h"
# include "gps.h"


namespace Buoy {
  Store::Store () {
    logf_id = 0;
    sd_data = NULL;
  }

  void Store::setup (BuoyMaster *b) // {{{
  {
    rf  = b->rf;
    ad  = b->ad;
    gps = b->gps;

    spi = new HardwareSPI(SD_SPI);

    lastsd    = millis ();
    lastbatch = 0;
    continuous_write = false;

    /* Initialize card */
    init ();

    rf->send_debug ("[SD] Store subsystem initiated.");
  } // }}}

  void Store::init () // {{{
  {
    rf->send_debug ("[SD] Init SD card.");
# if DIRECT_SERIAL
    SerialUSB.println ("[SD] Init SD card.");
# endif

    card    = new Sd2Card ();
    volume  = new SdVolume ();
    root    = new SdFile ();
    sd_data = new SdFile ();

    /* Initialize SD card */
    spi->begin (SPI_281_250KHZ, MSBFIRST, 0);
    SD_AVAILABLE = card->init (spi, SD_CS);

    if (SD_AVAILABLE)
      SD_AVAILABLE = card->cardSize() > 0;

    if (SD_AVAILABLE)
      SD_AVAILABLE = (card->errorCode () == 0);

    /* Beef up SPI speed after init is finished */
    if (SD_AVAILABLE)
      spi->begin (SPI_4_5MHZ, MSBFIRST, 0);

    /* Initialize FAT volume */
    if (SD_AVAILABLE)
      SD_AVAILABLE = volume->init (card, 1);

    /* Open root directory */
    if (SD_AVAILABLE)
      SD_AVAILABLE = root->openRoot (volume);

    if (SD_AVAILABLE)
    {
      rf->send_debug ("[SD] Card initialized.");

      open_index ();
      open_data ();

      //open_next_log ();

    } else {
      rf->send_debug ("[SD] [Error] Could not inititialize SD card.");
      current_index.id = 0;
    }
  } // }}}

  void Store::open_index () // {{{
  {
    if (!SD_AVAILABLE) return;

    uint32_t n = 0;

    rf->send_debug ("[SD] Opening index..");

    uint32_t i;
    SdFile fl;

    if (fl.open (root, "LASTID.LON", O_READ)) {

      n = fl.read (reinterpret_cast<char*>(&i), sizeof(uint32_t));
      if (n < sizeof(i)) i = 1;
      fl.close ();

    } else {
      i = 1;
    }

    rf_send_debug_f ("[SD] Last id: %lu..", i);
    SD_AVAILABLE &= (card->errorCode () == 0);
    next_index (i + 1);
  } // }}}

  /* Log to file {{{ */
  void Store::open_next_log ()
  {
    sprintf (buf, "%lu.LOG", logf_id);

    while (logf.open (root, buf, O_READ)) {
      logf.close ();
      logf_id++;
      sprintf (buf, "%lu.LOG", logf_id);
    }
    logf.close ();

    logf.open (root, buf, O_READ | O_CREAT | O_WRITE);
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

      sprintf (buf, "[%lu]", (uint32_t) gps->lastsecond);
      logf.write (buf);
      logf.write (s);
      logf.write ('\n');
    }
  } // }}}

  /* Open next index file */
  void Store::next_index (uint32_t i) // {{{
  {
    if (!SD_AVAILABLE) return;
    // i is LASTID or LASTID + 1

    // TODO: Handle better MAXID

    if (i > MAXID ) {
# if DIRECT_SERIAL
    SerialUSB.print ("[SD] Reached maximum ID:");
    SerialUSB.println (i);
    SerialUSB.println (MAXID);
# endif
      i = 1;  // DEBUG
    }

    /* Walk through subsequent indexes above lastid and take next free */
    bool newi = false;

# if DIRECT_SERIAL
    SerialUSB.print ("[SD] Checking for subseq. indexes from: ");
    SerialUSB.println (i);
# endif

    SdFile fi;

    while (!newi)
    {
      sprintf (buf, "%lu.IND", i);

      if (!fi.open(root, buf, O_READ)) {
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

      // TODO: Check if trying to open non-existent file sets errorCode()
      SD_AVAILABLE &= (card->errorCode () == 0);
      if (!SD_AVAILABLE) return;
    }
    i--;

    // Open new index
    current_index.version = STORE_VERSION;
    current_index.id = i;
    current_index.sample_l = SAMPLE_LENGTH;
    current_index.samples = 0;
    current_index.samples_per_reference = BATCH_LENGTH;
    current_index.nrefs = 0;

    SD_AVAILABLE &= (card->errorCode () == 0);
    write_index ();
  } // }}}

  void Store::write_index ()  // {{{
  {
    if (!SD_AVAILABLE) return;
    rf_send_debug_f ("[SD] Writing index: %lu..", current_index.id);

# if DIRECT_SERIAL
    SerialUSB.print   ("[SD] Writing index: ");
    SerialUSB.println (current_index.id);
# endif

    if (current_index.id != 0) {
      sprintf (buf, "%lu.IND", current_index.id);

      SdFile fi;
      fi.open (root, buf, O_CREAT | O_WRITE | O_TRUNC);
      fi.write (reinterpret_cast<char*>(&current_index.version), sizeof(current_index.version));
      fi.write (reinterpret_cast<char*>(&current_index.id), sizeof(current_index.id));
      fi.write (reinterpret_cast<char*>(&current_index.sample_l), sizeof(current_index.sample_l));
      fi.write (reinterpret_cast<char*>(&current_index.samples), sizeof(current_index.samples));
      fi.write (reinterpret_cast<char*>(&current_index.samples_per_reference), sizeof(current_index.samples_per_reference));
      fi.write (reinterpret_cast<char*>(&current_index.nrefs), sizeof(current_index.nrefs));

      for (uint32_t i = 0; i < current_index.nrefs; i++)
        fi.write (reinterpret_cast<char*>(&(current_index.refpos[i])), sizeof(uint32_t));

      for (uint32_t i = 0; i < current_index.nrefs; i++)
        fi.write (reinterpret_cast<char*>(&(current_index.refs[i])), sizeof(uint64_t));

      fi.sync ();
      fi.close ();

      /* Write back last index */
      SdFile fl;
      fl.open (root, "LASTID.LON", O_CREAT | O_WRITE | O_TRUNC);
      fl.write (reinterpret_cast<char*>(&(current_index.id)), sizeof(current_index.id));
      fl.sync ();
      fl.close ();
    }

    SD_AVAILABLE &= (card->errorCode () == 0);
  } // }}}

  /* Open new index and data file */
  void Store::roll_data_file () // {{{
  {
    if (!SD_AVAILABLE) return;
    rf->send_debug ("[SD] Syncing index and data and rolling..");

    /* Truncate data file to actual size */
    //sd_data.truncate (sd_data.curPosition ());

    sd_data->sync ();
    sd_data->close ();
    SD_AVAILABLE &= (card->errorCode () == 0);

    /* Write current index */
    write_index ();

    /* Open new index */
    next_index (current_index.id + 1);

    /* Open new data file */
    open_data ();
  } // }}}

  /* Write new batch of samples */
  void Store::write_batch () // {{{
  {
    if (!SD_AVAILABLE) {
      rf_send_debug_f ("[SD] No write: error: %02X.", card->errorCode ());
      return;
    }

    uint32_t s =  lastbatch * BATCH_LENGTH;

    /* Check if we have room for samples in store */
    if (current_index.samples > (MAX_SAMPLES_PER_FILE - BATCH_LENGTH))
    {
      roll_data_file ();
    }

    /* Check if we have room in size */
    if (sd_data->curPosition () > (SD_DATA_FILE_SIZE - (BATCH_LENGTH * (SAMPLE_LENGTH))))
    {
      roll_data_file ();
    }

    write_reference (ad->references[lastbatch], ad->reference_status[lastbatch]);

    if (!SD_AVAILABLE) return;

    /* Writing entries */
    rf_send_debug_f ("[SD] Writing entries to data file from sample: %lu", current_index.samples);

    /*
# if DIRECT_SERIAL
    SerialUSB.println ("[SD] Writing entries to data file.");
# endif
    */

    int r = sd_data->write (reinterpret_cast<char*>((uint32_t*) &(ad->values[s])), sizeof(uint32_t) * BATCH_LENGTH);

    if (r != sizeof(uint32_t) * BATCH_LENGTH) {
# if DIRECT_SERIAL
      SerialUSB.println ("[SD] [Error] Failed while writing samples.");
# endif
      rf->send_debug ("[SD] [Error] Card failed while writing samples.");

      SD_AVAILABLE = false;
    }

    current_index.samples += BATCH_LENGTH;

    sd_data->sync ();
    SD_AVAILABLE &= (card->errorCode () == 0);

    /* Ready for next batch if successful write */
    if (SD_AVAILABLE)
      lastbatch  =  (lastbatch + 1) % BATCHES;

    if (lastbatch != ad->batch) {
      rf->send_debug ("[SD] [Error] Did not finish writing batch before it was swapped.");
# if DIRECT_SERIAL
      SerialUSB.println ("[SD] [Error] Did not finish writing batch before it was swapped.");
# endif
    }
  } // }}}

  /* Open data file */
  void Store::open_data () // {{{
  {
    if (!SD_AVAILABLE) return;

    char fname[13];
    sprintf (fname, "%lu.DAT", current_index.id);

    SD_AVAILABLE &= sd_data->open (root, fname, O_CREAT | O_WRITE | O_TRUNC);
    SD_AVAILABLE &= (card->errorCode () == 0);
  } // }}}

  void Store::write_reference (uint64_t ref, uint32_t refstat) // {{{
  {
    rf_send_debug_f ("[SD] Write reference: %llu", ref);

    if (!SD_AVAILABLE) {
      rf_send_debug_f ("[SD] No write: error: %02X.", card->errorCode ());
      return;
    }

    /* Check if we have exceeded MAX_REFERENCES */
    if (current_index.nrefs >= (MAX_REFERENCES - 1)) {
      roll_data_file ();
    }

    /* Check if there is more space in data file */
    if (sd_data->curPosition () > (SD_DATA_FILE_SIZE - SD_REFERENCE_LENGTH)) {
      roll_data_file ();
    }

    /* Update index */
    current_index.refpos[current_index.nrefs] = sd_data->curPosition ();
    current_index.refs[current_index.nrefs]   = ref;

    /* Pad with 0 */
    for (uint32_t i = 0; i < (SD_REFERENCE_PADN * (SAMPLE_LENGTH)); i++)
      sd_data->write ((byte)0);

    sd_data->write (reinterpret_cast<char*>(&(current_index.nrefs)), sizeof(uint32_t));
    sd_data->write (reinterpret_cast<char*>(&(ref)), sizeof(uint64_t));
    sd_data->write (reinterpret_cast<char*>(&(refstat)), sizeof(uint32_t));

    /* Pad with 0 */
    for (uint32_t i = 0; i < (SD_REFERENCE_PADN * (SAMPLE_LENGTH)); i++)
      sd_data->write ((byte)0);

    current_index.nrefs++;

    SD_AVAILABLE &= (card->errorCode () == 0);
  } // }}}

  void Store::loop ()
  {
    /* Try to set up SD card, 5 sec delay  */
    if (!SD_AVAILABLE && (millis () - lastsd) > 5000) {
      rf_send_debug_f ("[SD] [Error] SD error code: %02X. Trying to reset.", card->errorCode ());

# if DIRECT_SERIAL
      SerialUSB.println ("[SD] [Error] Trying to reset.");
# endif

      /* Reset SPI */
      spi->end ();

      /* Clean up stale file references */
      if (card != NULL) {
        delete card;
        card = NULL;
      }

      if (volume != NULL) {
        delete volume;
        volume = NULL;
      }

      if (root != NULL) {
        delete root;
        root = NULL;
      }

      if (sd_data != NULL) {
        delete sd_data;
        sd_data = NULL;
      }

      send_id = 0;

      if (send_i != NULL) {
        delete send_i;
        send_i  = NULL;
      }

      if (send_d != NULL) {
        delete send_d;
        send_d  = NULL;
      }

      /* Reinitiate SD card */
      init ();

      lastsd = millis ();
    }

    /* Check if new batch is ready */
    /* Loop must run at least 2x speed (Nyquist) of batchfilltime */
    if (SD_AVAILABLE && continuous_write) {
      if (ad->batch != lastbatch) {
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

  /* ID list and retrieval */
  void Store::send_indexes (uint32_t start, uint32_t length) {
    if (!SD_AVAILABLE) {
      rf->send_error (RF::E_SDUNAVAILABLE);
      return;
    }

    /* Search on disk through indexes from start up to length nos */
    SdFile fi;
    while (length > 0) {
      sprintf (buf, "%lu.IND", start);
      if (!!fi.open (root, buf, O_READ)) {
        fi.close ();
        sprintf (rf->buf, "$IDS,%lu*", start);
        APPEND_CSUM(rf->buf);
        RF_Serial.println (rf->buf);
      }

      start++;
      length--;
    }
  }

  void Store::send_index (uint32_t id) {
    if (!SD_AVAILABLE) {
      rf->send_error (RF::E_SDUNAVAILABLE);
      return;
    }

    if (send_id != id || send_i == NULL) {
      if (send_i != NULL) {
        send_i->close ();
        delete send_i;
      }

      sprintf (buf, "%lu.IND", id);
      send_i = new SdFile ();
      if (!send_i->open (root, buf, O_READ)) {
        rf->send_error (RF::E_NOSUCHID);
        delete send_i;
        return;
      }
    }

    if (send_i->curPosition () > 0) send_i->seekSet (0);

    /* Reading first part of Index */
    uint32_t samples, samples_per_reference, nrefs, id_;
    uint16_t version, sample_l;

    send_i->read (reinterpret_cast<char*>(&version), sizeof(version));
    send_i->read (reinterpret_cast<char*>(&id_), sizeof(id_));
    send_i->read (reinterpret_cast<char*>(&sample_l), sizeof(sample_l));
    send_i->read (reinterpret_cast<char*>(&samples), sizeof(samples));
    send_i->read (reinterpret_cast<char*>(&samples_per_reference), sizeof(samples_per_reference));
    send_i->read (reinterpret_cast<char*>(&nrefs), sizeof(nrefs));

    // format: $IND,version,id,sample_l,samples,samples_per_reference,nrefs*CS
    sprintf(rf->buf, "$IND,%u,%lu,%u,%lu,%lu,%lu*", version, id_, sample_l, samples, samples_per_reference, nrefs);
    APPEND_CSUM (rf->buf);
    RF_Serial.println (rf->buf);
  }

  void Store::send_refs (uint32_t id, uint32_t start, uint32_t length) {
    /* Send references, _not_ reference positions */

  }

  void Store::send_batch (uint32_t id, uint32_t ref, uint32_t sample, uint32_t length) {
    if (!SD_AVAILABLE) {
      rf->send_error (RF::E_SDUNAVAILABLE);
      return;
    }
  }

  void Store::send_lastid () {
    if (!SD_AVAILABLE) {
      rf->send_error (RF::E_SDUNAVAILABLE);
      return;
    }

    sprintf (rf->buf, "$LID,%lu*", current_index.id);
    APPEND_CSUM (rf->buf);
    RF_Serial.println (rf->buf);
  }
}

/* vim: set filetype=arduino :  */

