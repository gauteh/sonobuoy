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
      Bdata bdata;
      bool  ready;

      Dtt (int);

      bool read_index ();    // Read ITT and load all
      bool read_batches ();  // Load batches with samples

      ~Dtt ();
  };
}

