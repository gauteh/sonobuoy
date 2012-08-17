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

# define SAMPLERATE   250.0
# define BATCHLENGTH  1024

using namespace std;

namespace Zero {
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
      // HasFull..

      int totalsamples;

      vector<Batch> batches;

      /* Quality and fixing */
      bool fixedtime;
      bool notimefix;
      bool checksum_passed;

      void check_checksums ();
      void populate_int32_samples ();
      void fix_time ();
      void assess_dataquality ();

  };
}

