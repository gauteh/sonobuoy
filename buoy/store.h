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
# include "gps.h"
# include "ad7710.h"

# define SD_SS    10
# define SD_MOSI  11
# define SD_MISO  12
# define SD_SCK   13

extern SdFat  sd;
extern bool   SD_AVAILABLE;
extern uint    sd_error;

void sd_setup ();
void sd_loop ();

void open_index ();
void write_index ();

/* Data format */
# define STORE_VERSION 1
# define SAMPLE_LENGTH 3
# define TIMESTAMP_LENGTH 4

/* Maximum number of timestamp, sample pairs for each datafile */
# define EST_MINUTES_PER_DATAFILE 10L
# define MAX_SAMPLES_PER_FILE (EST_SAMPLE_RATE * 60L * EST_MINUTES_PER_DATAFILE)
# define MAX_REFERENCES (MAX_SAMPLES_PER_FILE / ( EST_SAMPLE_RATE * ROLL_REFERENCE))

/* Last ID is one unsigned long */
typedef ulong LASTID;

typedef struct _Index {
  uint version;     // Version of data (as defined in STORE_VERSION)
  ulong id;          // Id of index

  uint sample_l;    // Length of sample (bytes)
  uint timestamp_l; // Length of time stamp (bytes)

  ulong samples;    // Can maximum reach MAX_SAMPLES_PER_FILE
  ulong nrefs;      // Current number of references
  ulong refs[MAX_REFERENCES]; // List with position of reference points.

  bool closed;      // Indicates whether this index has been closed

} Index;


typedef struct _DataHeader {
  uint index;           // Corresponding index file
  uint id;              // Data file id

  uint samplelength;

  uint firstreference;  // First reference time stamp in data
} DataHeader;

/* File names:
 * LASTID.DAT    - file with current index id (not to be trusted..)
 *
 * INDEX[id].IND - Index of data files, highest id is current.
 *
 * A new Index will be created after a number of data files or
 * if index cannot be parsed.
 *
 * DATA[id].DAT -  Data related to INDEX with same id
 *
 * A new data file will be created on start up, if file is corrupt, on
 * new index or if file is corrupt.
 */



# endif

/* vim: set filetype=arduino :  */

