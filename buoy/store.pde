/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-06
 *
 * Handle local SD card storage
 *
 */

# include <SdFat.h>

# include "store.h"
# include "rf.h"
# include "ad7710.h"

SdFat sd;
bool SD_AVAILABLE = false;
ulong sd_status = 0;
volatile bool update_reference = false;
volatile uint update_reference_qposition = 0;

ulong lastsd = 0;

/* Current storage */
Index current_index;
SdFile sd_data;

void sd_setup () {
  pinMode (SD_SS, OUTPUT);
  pinMode (SD_MOSI, OUTPUT);
  pinMode (SD_MISO, INPUT);
  pinMode (SD_SCK, OUTPUT);

  lastsd = millis ();
  sd_init ();
}

# if DIRECT_SERIAL
ArduinoOutStream cout(Serial);
# endif

void sd_init ()
{
  SD_AVAILABLE = (sd.init (SPI_FULL_SPEED, SD_SS) & (sd.card()->cardSize() > 0));
  SD_AVAILABLE &= (sd.card()->errorCode () == 0);

  if (SD_AVAILABLE)
  {
    rf_send_debug ("SD card ready.");

    sd.chdir ("/", true);
    sd_open_index ();
    sd_open_data ();

  } else {
    rf_send_debug ("Could not init SD.");
    current_index.id = 0;
  }
}

void sd_open_index ()
{
  int n = 0;

  rf_send_debug ("Opening index..");

  ulong i;
  if (sd.exists("LASTID.LON")) {
    SdFile fl ("LASTID.LON", O_READ);

    n = fl.read (reinterpret_cast<char*>(&i), sizeof(ulong));

    if (n < sizeof(i)) i = 1;

    fl.close ();
  } else {
    i = 1;
  }

  rf_send_debug_f ("Last id: %lu..", i);
  sd_next_index (i);
}

/* Open next index file */
void sd_next_index (ulong i)
{
  // i is LASTID

  // TODO: Check if we have reached MAXID

  if (i > 100) i = 1;  // DEBUG

  /* Walk through subsequent indexes above lastid and take next free */
  int n = 0;
  char buf[8+5];
  bool newi = false;

# if DIRECT_SERIAL
  Serial.print ("Checking for subseq. indexes from: ");
  Serial.println (i);
# endif

  while (!newi)
  {
    sprintf (buf, "%lu.IND", i);
# if DIRECT_SERIAL
    Serial.println(buf);
# endif

    if (!sd.exists(buf)) {
      newi = true; /* Found new index file at id I */

# if DIRECT_SERIAL
      Serial.print ("Found free index at: ");
      Serial.println (i);
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

  sd_write_index ();
}

void sd_write_index ()
{
  char buf[8+5];
  rf_send_debug_f ("Writing index: %lu..", current_index.id);

  if (current_index.id != 0) {
    sprintf (buf, "%lu.IND", current_index.id);

    SdFile fi (buf, O_CREAT | O_WRITE | O_TRUNC);
    fi.write (reinterpret_cast<char*>(&current_index), sizeof(current_index));
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
void sd_roll_data_file ()
{
  rf_send_debug ("Syncing index and data and rolling..");

  /* Truncate data file to actual size */
  //sd_data.truncate (sd_data.curPosition ());

  sd_data.sync ();
  sd_data.close ();

  /* Open new index */
  sd_write_index ();
  sd_next_index (current_index.id);

  /* Open new data file */
  sd_open_data ();
}

bool referencewritten = false;

/* Write new batch of samples */
void sd_write_batch ()
{
  if (!SD_AVAILABLE) {
    rf_send_debug_f ("No write: error: %02X.", sd.card ()->errorCode ());
    return;
  }

  /* Check if we have room for samples in store */
  if (current_index.samples > (MAX_SAMPLES_PER_FILE - (AD_QUEUE_LENGTH / 2)))
  {
    sd_roll_data_file ();
  }


  /* Check if we have exceeeded this store */
  if (sd_data.curPosition () > (SD_DATA_FILE_SIZE - (AD_QUEUE_LENGTH / 2 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH))))
  {
    sd_roll_data_file ();
  }

  /* In case we are in update_reference, check if we have space for
   * one more reference */
  if (update_reference) {
    if (sd_data.curPosition () > (SD_DATA_FILE_SIZE - (AD_QUEUE_LENGTH / 2 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)) - SD_REFERENCE_LENGTH))
    {
      sd_roll_data_file ();
    }
  }

  /* Write first reference since this is a new file */
  if (!referencewritten) {
    sd_write_reference (referencesecond);
  }

  /* Writing entries */
  rf_send_debug_f ("Writing entries to data file..: %lu", current_index.samples);
  uint i = (batchready == 1 ? 0 : (AD_QUEUE_LENGTH / 2));
  for (; i <  (AD_QUEUE_LENGTH / 2); i++)
  {
    if (update_reference && i == update_reference_qposition) {
      sd_write_reference (referencesecond);
      update_reference = false;
    }

    sd_data.write (reinterpret_cast<char*>((ulong*) &(ad_time[i])), sizeof(ulong));
    sd_data.write (reinterpret_cast<char*>((byte*) &(ad_queue[i])), sizeof(sample));
    current_index.samples++;
  }

  sd_data.sync ();
  SD_AVAILABLE &= (sd.card()->errorCode () == 0);
}

/* Open data file */
void sd_open_data ()
{
  char fname[13];
  sprintf (fname, "%lu.DAT", current_index.id);

  SD_AVAILABLE = sd_data.open (fname, O_CREAT | O_WRITE | O_TRUNC);
  SD_AVAILABLE &= (sd.card()->errorCode () == 0);

  referencewritten = false;
}

void sd_write_reference (ulong ref)
{
  rf_send_debug ("Write reference.");
  if (SD_AVAILABLE)
  {
    /* Check if we have exceeded MAX_REFERENCES */
    if (current_index.nrefs >= (MAX_REFERENCES - 1)) {
      sd_roll_data_file ();
    }

    /* Check if there is more space in data file */
    if (sd_data.curPosition () > (SD_DATA_FILE_SIZE - SD_REFERENCE_LENGTH)) {
      sd_roll_data_file ();
    }

    /* Update index */
    current_index.refs[current_index.nrefs] = sd_data.curPosition ();

    /* Pad with 0 */
    for (int i = 0; i < (3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); i++)
      sd_data.write ((byte)0);

    sd_data.write (reinterpret_cast<char*>(&(current_index.nrefs)), sizeof(ulong));
    sd_data.write (reinterpret_cast<char*>(&(ref)), sizeof(ulong));
    sd_data.write (reinterpret_cast<char*>(&(sd_status)), sizeof(ulong));

    /* Pad with 0 */
    for (int i = 0; i < (3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); i++)
      sd_data.write ((byte)0);

    current_index.nrefs++;
    referencewritten = true;

    SD_AVAILABLE &= (sd.card()->errorCode () == 0);
  }
}

void sd_loop ()
{
  /* Try to set up SD card, 5 sec delay  */
  if (!SD_AVAILABLE & (millis () - lastsd) > 5000) {
    rf_send_debug_f ("SD error code: %02X.", sd.card ()->errorCode ());
    sd_init ();
    lastsd = millis ();
  }
}

/* vim: set filetype=arduino :  */

