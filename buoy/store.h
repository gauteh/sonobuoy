/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-06
 *
 * Handle local SD card storage
 *
 */

# ifndef STORE_H
# define STORE_H

# include "buoy.h"
# include "gps.h"
# include "ad7710.h"

# ifndef ONLY_SPEC
# include <SdFat.h>

# define SD_SS    10
# define SD_MOSI  11
# define SD_MISO  12
# define SD_SCK   13

extern SdFat  sd;
extern bool   SD_AVAILABLE;

extern ulong  sd_status;

# endif /* ONLY_SPEC */

enum SD_STATUS {
  SD_VALID_GPS = 0x1,
  SD_HAS_TIME  = 0x2,
  SD_HAS_SYNC  = 0x4,
  SD_HAS_SYNC_REF = 0x8,
};

# ifndef ONLY_SPEC
/* Should be set when new reference has become available, will be written
 * outside of interrupt since it might otherwise come in the middle of a
 * writing operation */
extern volatile bool update_reference;
extern volatile uint update_reference_qposition;

void sd_setup ();
void sd_loop ();
void sd_init ();

void sd_open_index ();
void sd_write_index ();
void sd_next_index (int);
void sd_roll_data_file ();
void sd_open_data ();

void sd_write_batch ();
void sd_write_reference (ulong);

# endif /* ONLY_SPEC */

/* Data format */
# define STORE_VERSION 1
# define SAMPLE_LENGTH 3
# define TIMESTAMP_LENGTH 4

/* Maximum number of timestamp, sample pairs for each datafile */
# define EST_MINUTES_PER_DATAFILE 2L
# define MAX_SAMPLES_PER_FILE (EST_SAMPLE_RATE * 60L * EST_MINUTES_PER_DATAFILE)
# define MAX_REFERENCES (MAX_SAMPLES_PER_FILE / ( EST_SAMPLE_RATE * ROLL_REFERENCE))

# define _SD_DATA_FILE_SIZE (MAX_SAMPLES_PER_FILE * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) + MAX_REFERENCES * 50)
# define SD_DATA_FILE_SIZE (_SD_DATA_FILE_SIZE + (_SD_DATA_FILE_SIZE % 512))

/* Data file format {{{
 *
 * Reference:
 *  - 3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) with 0
 *  - Reference id: ulong
 *  - Reference:    ulong referencesecond [unix time]
 *  - Status bit:   ulong status
 *  - 3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) with 0
 *  Total length: 54 bytes.
 *
 * Entry:
 *  - TIMESTAMP (4 bytes)
 *  - SAMPLE    (3 bytes)
 *  Total length: 7 bytes.
 *
 * }}} */

# define SD_REFERENCE_LENGTH 54

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
} Index;


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

