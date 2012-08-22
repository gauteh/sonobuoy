/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-22
 *
 * dat.h: Interface to DAT file
 *
 */

# pragma once

# include <stdint.h>

# include "bdata.h"
# define ONLY_SPEC
    typedef char byte;
# include "store.h"

using namespace std;

namespace Zero {
  class Dat {
    public:
      Bdata *bdata;
      bool  ready;

      Dat (int);
      Dat (Bdata *);

      bool read_index ();    // Read IND and load all
      bool read_batches ();  // Load batches with samples

      /*
      void write (const char *, const char *); // does both of the below
      void write_index (const char *);    // write index file
      void write_batches (const char *);  // write dtt file
      */

      ~Dat ();
  };
}

