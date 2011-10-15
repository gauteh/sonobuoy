/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-06
 *
 * Handle local SD card storage
 *
 */

# include <SdFat.h>

# include "store.h"
# include "rf.h"

SdFat sd;
uint  sd_error = 0;
bool SD_AVAILABLE = false;

bool lastidloaded = false;
ulong lastsd = 0;

/* Current storage index */
Index current_index;

void sd_setup () {
  pinMode (SD_SS, OUTPUT);
  pinMode (SD_MOSI, OUTPUT);
  pinMode (SD_MISO, INPUT);
  pinMode (SD_SCK, OUTPUT);

  lastsd = millis ();
  init_sd ();
}

# if DIRECT_SERIAL
ArduinoOutStream cout(Serial);
# endif

void init_sd ()
{
  SD_AVAILABLE = (sd.init (SPI_HALF_SPEED, SD_SS) & (sd.card()->cardSize() > 0));

  if (SD_AVAILABLE)
  {
    rf_send_debug ("SD card ready.");

    sd.chdir ("/", true);
    open_index ();

  } else {
    rf_send_debug ("Could not init SD.");
    current_index.id = 0;
  }
}

void open_index ()
{
  int n = 0;

  delayMicroseconds (100);

  rf_send_debug ("Opening index..");

  ulong i;
  SdFile fl ("LASTID.DAT", O_READ);

  n = fl.read (reinterpret_cast<char*>(&i), sizeof(ulong));
  Serial.println (n);
  
  fl.close ();

  char buf[50];
  sprintf (buf, "Last id: %lu..", i);
  rf_send_debug (buf);

  if (i > 1000) i = 1; // Reset last id to 1 if none could be read
  lastidloaded = true;

  /* Walk through subsequent indexes above lastid and take next free */

  bool newi = false;

  Serial.print ("Checking for subseq. indexes from: ");
  Serial.println (i);

  while (!newi)
  {
    sprintf (buf, "INDEX%lu.IND", i);
    Serial.println(buf);

    if (!sd.exists(buf)) {
      Serial.print ("Found free index at: ");
      Serial.println (i);
      newi = true; /* Found new index file at id I */
    }
    i++;
  }
  i--;

  if (i > 1) {

    /* Change to previous existing index */
    i = i - 1;

    /* Open previous index and check if it is closed */
    sprintf (buf, "INDEX%lu.IND", i);

    if (sd.exists(buf)) {

      SdFile fi (buf, O_READ);
      n = fi.read(reinterpret_cast<char*>(&current_index), sizeof(current_index));
      fi.close ();

      /* Could not fully read index, skip */
      if (n != sizeof(current_index)) {
        current_index.closed = true;
      }

    } else {
      /* No index file found, start new at previous index (can happen if there
       * are jumps in LASTID) */
      current_index.closed = true;
      i = i - 1;
    }

  } else {
    /* Starting first index */
    current_index.closed = true;
    i = 0; // Will be incremented below.
  }


  if (current_index.closed) {
    // Open new index
    current_index.version = STORE_VERSION;
    current_index.id = i + 1;
    current_index.datafiles = 0;
    current_index.sample_l = SAMPLE_LENGTH;
    current_index.timestamp_l = TIMESTAMP_LENGTH;
    current_index.closed = false;
  }
  write_index ();
}

void write_index ()
{
  char buf[50];
  sprintf (buf, "Writing index: %lu..", current_index.id);
  rf_send_debug (buf);

  if (current_index.id != 0) {
    sprintf (buf, "/INDEX%lu.IND", current_index.id);

    /* DEBUG*/
    current_index.closed = true;

    SdFile fi (buf, O_CREAT | O_WRITE | O_TRUNC);
    fi.write (reinterpret_cast<char*>(&current_index), sizeof(current_index));
    fi.sync ();
    fi.close ();

    /* Write back last index */
    SdFile fl("LASTID.DAT", O_CREAT | O_WRITE | O_TRUNC);
    fl.write (reinterpret_cast<char*>(&(current_index.id)), sizeof(current_index.id));
    fl.sync ();
    fl.close ();
  }
# if DIRECT_SERIAL
  sd.ls ();
# endif
}

void next_file ()
{

}

void roll_file ()
{

}

ulong lasttry = 0;

void sd_loop ()
{
  /* Check if everything is happy dandy with card.. */

  /* Try to set up SD card, 5 sec delay  */
  if (!SD_AVAILABLE & (millis () - lastsd) > 5000) {
    init_sd ();
    lastsd = millis ();
  }
}

/* vim: set filetype=arduino :  */

