/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * bdata.h: Data structure for id with batches
 *
 */

# pragma once

# include <stdint.h>
# include <vector>
# include <string>
# include <libmseed/libmseed.h>

# define SAMPLERATE   250.0
# define BATCHLENGTH  1024
# define BATCHES      40

# define GOODSTATUS   15u

using namespace std;

namespace Zero {
  /* Sane limits for checking time */
  static hptime_t MAXTIME ();
  static hptime_t MINTIME ();

  class Bdata {
    public:
      /* Batch */
      typedef struct _Batch {
        uint32_t  length;
        uint32_t  no;
        uint64_t  ref; // timestamp
        uint16_t  status;
        string    latitude;
        string    longitude;
        uint32_t  checksum;
        // finished chunks..

        /* Quality and fixing */
        bool hasclipped;    // has full scale range been exceeded
        bool checksum_pass; // Did checksum validate
        char dataquality;   // Overall quality indicator

        bool fixedtime;     // Did we have to fix time
        bool notimefix;     // Could not fix time
        uint64_t origtime;

        uint32_t *samples_u;
        int32_t  *samples_i;
      } Batch;

      /* Bdata */
      int localversion;
      int remoteversion;
      int id;
      int samplescount;
      int batchcount;
      bool hasfull;
      bool e_sdlag;

      int totalsamples;

      vector<Batch> batches;

      /* Quality and fixing */
      bool fixedtime;       // time has been fixed on one or more batches
      bool notimefix;       // no batch could be used to fix times, all are bad
      bool checksum_passed; // all checksums passed

      void check_checksums ();
      void populate_int32_samples ();
      void fix_batch_time ();
      void assess_dataquality ();

  };

  class Collection {
    public:
      Collection ();

      vector<Bdata> datas;

      bool fixedtime;
      bool notimefix;

      /* Fix times */
      void fix_data_time ();
  };
}

