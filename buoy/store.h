/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-02-05
 *
 * SD store
 *
 */

# pragma once

# include "buoy.h"

# include "ads1282.h"

# ifndef ONLY_SPEC

# include "wirish.h"
# include "HardwareSPI.h"
# include "SdFat.h"

# include "types.h"

/* SDIO, connected to SD socket. */
# if BBOARD == 0
/* SPI mode, pins:
 * CS     = CD (card detect) / SDIO_D3 (Data 3)
 * DI     = SDIO_CMD
 * SCLK   = SDIO_CK
 * DO     = SDIO_D0 (Data 0)
 */
# define SDIO_D0   98
# define SDIO_D1   99
# define SDIO_D2  111
# define SDIO_D3  112

# define SDIO_CK  113
# define SDIO_CMD 116

/* SPI 2
 *
 * NSS  = 2
 * SCK  = 3
 * MISO = 4
 * MOSI = 5
 */
# define SD_SPI 2
# define SD_CS  2

# elif BBOARD == 1

/* SPI 2
 *
 * NSS  = 25
 * SCK  = 32
 * MISO = 33
 * MOSI = 34
 */
# define SD_SPI 2
# define SD_CS  25 // chip select pin

# endif

namespace Buoy {
  class Store {
    public:
# if HASRF
      RF      *rf;
# endif
      ADS1282 *ad;

      HardwareSPI *spi;
      Sd2Card     *card;
      SdVolume    *volume;
      SdFile      *root;

      char buf[8+5]; // used for file names

      bool      SD_AVAILABLE;
      uint32_t  lastsd;

# endif

      typedef uint32_t sample;

/* Data format */
# define STORE_VERSION 5
# define SAMPLE_LENGTH 4

/* Data file format {{{
 *
 * Reference:
 *  - 3 * (SAMPLE_LENGTH) with 0
 *  - Reference id: uint32_t
 *  - Reference:    uint64_t referencesecond [unix time + microdelta]
 *  - Status bit:   uint32_t status
 *  - Latitude:     uint16_t (first bit positive: north)
 *  - Longitude:    uint16_t (first bit positive: east)
 *  - 3 * (SAMPLE_LENGTH) with 0
 *  Total length: 40 bytes.
 *
 * Entry:
 *  - SAMPLE    (4 bytes)
 *  Total length: 4 bytes.
 *
 * }}} */

# define SD_REFERENCE_PADN 3
# define SD_REFERENCE_LENGTH (2 * 3 * (SAMPLE_LENGTH) + 4 * 4 + 2 * 2)

# define MAX_SAMPLES_PER_FILE (BATCH_LENGTH * 40)
# define MAX_REFERENCES (MAX_SAMPLES_PER_FILE / BATCH_LENGTH)

/* Maximum number of bytes in each data file */
# define SD_DATA_FILE_SIZE (MAX_SAMPLES_PER_FILE * SAMPLE_LENGTH + MAX_REFERENCES * SD_REFERENCE_LENGTH)

      /* Last ID is one unsigned long */
      typedef uint32_t LASTID;

      typedef struct _Index {
        uint16_t version;     // Version of data (as defined in STORE_VERSION)
        uint32_t id;          // Id of index (limited by MAXID)

        uint16_t sample_l;    // Length of sample (bytes)

        uint32_t samples;     // Can maximum reach MAX_SAMPLES_PER_FILE
        uint32_t samples_per_reference; // Is defined by BATCH_LENGTH
        uint32_t nrefs;       // Current number of references
      } Index;

/* Using 8.3 file names limits the ID */
# define MAXID (100000000L -1L)

      /* Files:
       * LASTID.LON     - file with current index id (not to be trusted..)
       *
       * [id].IND       - Index of data files, highest id is current.
       *
       * A new Index will be created after a number of data files or
       * if index cannot be parsed.
       *
       * [id].DAT       -  Data related to INDEX with same id
       *
       * A new data file will be created on start up, if file is corrupt, on
       * new index or if file is corrupt.
       */

/* Log files */
# define MAX_LOG_SIZE (1024 * 1024 * 1)

# ifndef ONLY_SPEC

      /* Current storage */
      Index current_index;
      SdFile *sd_data;

      /* Logging */
      uint32_t logf_id;
      SdFile logf;

      bool     continuous_write;
      uint32_t lastbatch;

      Store ();

      void setup (BuoyMaster *);
      void loop ();
      void init ();

      void open_index ();
      void write_index ();
      void next_index (uint32_t);
      void roll_data_file ();
      void open_data ();

      void write_batch ();
      void write_reference (uint64_t, uint32_t, uint16_t, uint16_t);

      void start_continuous_write ();
      void stop_continuous_write ();

      /* ID and files currently being sent */
# define GET_IDS_N 10 // no of ids to send in one go
# if HASRF
      uint32_t s_id;
      uint32_t s_samples;
      uint32_t s_nrefs;
      uint16_t s_lastbatch;
      SdFile *send_i;
      SdFile *send_d;

      void _reset_index ();
      bool _check_index (uint32_t); // check if current index is open and valid

      void send_indexes (uint32_t, uint32_t);
      void send_index (uint32_t);
      void send_batches (uint32_t id, uint32_t refno, uint32_t start, uint32_t length);
      void send_lastid ();
# endif

# if 0
      void open_next_log ();
      void log (const char *);
# endif
  };
}

# endif

/* vim: set filetype=arduino :  */

