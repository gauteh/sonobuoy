/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-02-05
 *
 * SD store
 *
 */

# pragma once

# include "ads1282.h"
# include "gps.h"
# include "buoy.h"

# ifndef ONLY_SPEC

# include "wirish.h"
# include "HardwareSPI.h"
# include "types.h"
# include "SdFat.h"

/* SDIO, connected to SD socket. */

# define SDIO_D0   98
# define SDIO_D1   99
# define SDIO_D2  111
# define SDIO_D3  112

# define SDIO_CK  113
# define SDIO_CMD 116


/* SPI mode, pins:
 * CS     = CD (card detect) / SDIO_D3 (Data 3)
 * DI     = SDIO_CMD
 * SCLK   = SDIO_CK
 * DO     = SDIO_D0 (Data 0)
 */
/*
# define SD_CS    SDIO_D3
# define SD_MOSI  SDIO_CMD
# define SD_MISO  SDIO_D0
# define SD_SCLK  SDIO_CK
*/

/* SPI 2
 *
 * NSS  = 2
 * SCK  = 3
 * MISO = 4
 * MOSI = 5
 */
# define SD_SPI 2
# define SD_CS  2



namespace Buoy {
  class Store {
    public:
      RF      *rf;
      ADS1282 *ad;
      GPS     *gps;

      HardwareSPI *spi;
      Sd2Card     card;
      SdVolume    volume;
      SdFile      root;

      bool  SD_AVAILABLE;
      uint32_t lastsd;

# endif

      typedef uint32_t sample;

/* Data format */
# define STORE_VERSION 3uL
# define SAMPLE_LENGTH 4uL
# define TIMESTAMP_LENGTH 4uL

/* Maximum number of timestamp, sample pairs for each datafile */
# define MINUTES_PER_DATAFILE 5uL
# define MAX_SAMPLES_PER_FILE (FREQUENCY * 60uL * MINUTES_PER_DATAFILE)
# define MAX_REFERENCES (MAX_SAMPLES_PER_FILE / BATCH_LENGTH)


# define _SD_DATA_FILE_SIZE (MAX_SAMPLES_PER_FILE * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) + MAX_REFERENCES * 50uL)
# define SD_DATA_FILE_SIZE (_SD_DATA_FILE_SIZE + (_SD_DATA_FILE_SIZE % 512uL))

/* Data file format {{{
 *
 * Reference:
 *  - 3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) with 0
 *  - Reference id: uint32_t
 *  - Reference:    uint32_t referencesecond [unix time]
 *  - Status bit:   uint32_t status
 *  - 3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) with 0
 *  Total length: 54 bytes.
 *
 * Entry:
 *  - TIMESTAMP (4 bytes)
 *  - SAMPLE    (4 bytes)
 *  Total length: 8 bytes.
 *
 * }}} */

# define SD_REFERENCE_PADN 3
# define SD_REFERENCE_LENGTH (2 * 3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH) + 3 * 4)

      /* Last ID is one unsigned long */
      typedef uint32_t LASTID;

      typedef struct _Index {
        uint16_t version;     // Version of data (as defined in STORE_VERSION)
        uint32_t id;          // Id of index (limited by MAXID)

        uint16_t sample_l;    // Length of sample (bytes)
        uint16_t timestamp_l; // Length of time stamp (bytes)

        uint32_t samples;     // Can maximum reach MAX_SAMPLES_PER_FILE
        uint32_t nrefs;       // Current number of references
        uint32_t refs[MAX_REFERENCES]; // List with position of reference points.
      } Index;


/* Using 8.3 file names limits the ID */
# define MAXID ((10^8uL) -1uL)

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
      SdFile sd_data;

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
      void write_reference (uint32_t, uint32_t);

      void start_continuous_write ();
      void stop_continuous_write ();

      void open_next_log ();
      void log (const char *);
  };
}

# endif

/* vim: set filetype=arduino :  */

