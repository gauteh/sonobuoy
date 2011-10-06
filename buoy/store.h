/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-06
 *
 * Handle local SD card storage
 *
 */

# ifndef STORE_H
# define STORE_H

# include <SdFat.h>
# include "buoy.h"

# define SD_SS    10
# define SD_MOSI  11
# define SD_MISO  12
# define SD_SCK   13

extern SdFat sd;
extern bool SD_AVAILABLE;

void sd_setup ();
void sd_loop ();

void open_index ();
void write_index ();

/* Data format */
# define STORE_VERSION 1
# define SAMPLE_LENGTH 3
# define TIMESTAMP_LENGTH 4

typedef struct _Lastid {
  uint id;
} Lastid;

typedef struct _Index {
  uint version;     // Version of data (as defined in STORE_VERSION)
  uint id;          // Id of index
  uint datafiles;   // Number of data files

  uint sample_l;    // Length of sample (bytes)
  uint timestamp_l; // Length of timestamp (bytes)

  bool closed;      // Indicates whether this index has been closed

} Index;

typedef struct _DataHeader {
  uint index;           // Corresponding index file
  uint id;              // Data file id

  uint samplelength;

  uint firstreference;  // First reference time stamp in data
} DataHeader;

/* File names:
 * LASTID    - file with current index id (not to be trusted..)
 *
 * INDEX[id] - Index of data files, highest id is current.
 * 
 * A new Index will be created after a number of data files or
 * if index cannot be parsed.
 *
 * DATA[indexid]_[id] - Number of corresponding index as well as
 *                      id of data file.
 * 
 * A new data file will be created on start up, if file is corrupt, on
 * new index or if file is corrupt.
 */



# endif

/* vim: set filetype=arduino :  */

