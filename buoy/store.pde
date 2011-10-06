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
char sd_error[80];
bool SD_AVAILABLE = false;

/* Storage format */
Index current_index;

void sd_setup () {
  pinMode (SD_SS, OUTPUT);
  pinMode (SD_MOSI, OUTPUT);
  pinMode (SD_MISO, INPUT);
  pinMode (SD_SCK, OUTPUT);

  init_sd ();
}

void init_sd ()
{
  SD_AVAILABLE = sd.init (SPI_HALF_SPEED);

  if (SD_AVAILABLE)
  {
    sd.chdir (true); // Go to root

    open_index ();
  }
}

void open_index ()
{
  Lastid l;

  if (sd.exists ("LASTID"))
  {
    SdFile fl;
    fl.open ("LASTID", O_READ);
    fl.read (&l, sizeof(l));
    fl.close ();
  } else {
    l.id = 0;
  }

  /* Walk through subsequent indexes and check if there are any above 
   *
   * If there are gaps in numbering subsequent indexes WILL be overwritten. */

  bool newi = false;
  uint i = l.id;

  char buf[50];

  while (!newi)
  {
    sprintf (buf, "INDEX%u", i);
    if (!sd.exists(buf)) {
      newi = true;
    }
    i++;
  }
  i--;

  /* Open latest index and check if it is closed */
  SdFile fi;
  fi.open (buf, O_READ);
  fi.read (&current_index, sizeof(current_index));
  fi.close ();

  if (current_index.closed) {
    // Open new index
    i++;
    current_index.version = STORE_VERSION;
    current_index.id = i;
    current_index.datafiles = 0;
    current_index.sample_l = SAMPLE_LENGTH;
    current_index.timestamp_l = TIMESTAMP_LENGTH;
    current_index.closed = false;
    write_index ();
  }
}

void write_index ()
{
  if (SD_AVAILABLE) {
    if (current_index.id != 0) {
      SdFile fi;
      char buf[50];
      sprintf (buf, "INDEX%u", current_index.id);

      fi.open (buf, O_CREAT | O_WRITE | O_TRUNC);
      fi.write (&current_index, sizeof(current_index));
      fi.sync ();
      fi.close ();
      
      /* Write back last index */
      SdFile fl;
      fl.open ("LASTID", O_CREAT | O_WRITE | O_TRUNC);
      Lastid l = { current_index.id };
      fl.write (&l, sizeof (l));
      fl.sync ();
      fl.close ();
    }
  }
}

void next_file ()
{

}

void roll_file ()
{

}

void sd_loop ()
{
  /* Check if everything is happy dandy with card.. */

  /* Try to set up SD card */
  if (!SD_AVAILABLE)
    init_sd ();
}

/* vim: set filetype=arduino :  */

