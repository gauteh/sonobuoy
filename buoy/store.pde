/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-06
 *
 * Handle local SD card storage
 *
 */

# include <SdFat.h>

# include "store.h"

SdFat sd;
char sd_error[80];
bool SD_AVAILABLE = false;

void sd_setup () {
  pinMode (SD_SS, OUTPUT);
  pinMode (SD_MOSI, OUTPUT);
  pinMode (SD_MISO, INPUT);
  pinMode (SD_SCK, OUTPUT);

  SD_AVAILABLE = sd.init (SPI_HALF_SPEED);

  sd.chdir (true);
  sd.mkdir ("/TESTDIR", true);

  SdFile f;
  SD_AVAILABLE = f.open ("/test.txt", O_CREAT | O_WRITE);
  f.write ("Testing..");
  f.sync ();
  f.close ();
}

void sd_loop ()
{

}

/* vim: set filetype=arduino :  */

