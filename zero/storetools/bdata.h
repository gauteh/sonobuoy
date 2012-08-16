/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * bdata.h: Data structure for id with batches
 *
 */

# pragma once

# include <stdint.h>
# include <vector>

# define SAMPLERATE 250.0

using namespace std;

namespace Zero {
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

    int32_t *samples;
  } Batch;

  /* Bdata */
  typedef struct _Bdata {
    int localversion;
    int remoteversion;
    int id;
    int samplescount;
    int batchcount;
    // HasFull..

    int totalsamples;

    vector<Batch> batches;
  } Bdata;
}

