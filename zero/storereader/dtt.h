/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * dtt.h: Interface to DTT file
 *
 */

# pragma once

# include <stdint.h>
# include <vector>

# define BATCH_LENGTH 1024

using namespace std;

namespace Zero {
  class Dtt {
    public:
      /* Batch */
      typedef struct Batch {
        uint32_t  length;
        uint32_t  no;
        uint64_t  ref; // timestamp
        uint16_t  status;
        string    latitude;
        string    longitude;
        uint32_t  checksum;

        // finished chunks..

        int32_t *samples;
      } Batch;

      /* Index */
      int localversion;
      int remoteversion;
      int id;
      int samplescount;
      int batchcount;
      // HasFull..

      int totalsamples;

      vector<Batch> batches;

      Dtt (int);

      void read_index ();    // Read ITT and load all
      void read_batches ();  // Load batches with samples

      ~Dtt ();
  };
}

