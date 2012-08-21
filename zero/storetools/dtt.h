/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * dtt.h: Interface to DTT file
 *
 */

# pragma once

# include <stdint.h>

# include "bdata.h"

# define BATCH_LENGTH 1024

using namespace std;

namespace Zero {
  class Dtt {
    public:
      Bdata *bdata;
      bool  ready;

      Dtt (int);
      Dtt (Bdata *);

      bool read_index ();    // Read ITT and load all
      bool read_batches ();  // Load batches with samples

      void write (const char *, const char *); // does both of the below
      void write_index (const char *);    // write index file
      void write_batches (const char *);  // write dtt file

      ~Dtt ();
  };
}

