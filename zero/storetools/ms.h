/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * ms.h: Interface to libmseed designed for buoy
 *
 */

# pragma once

# include <stdint.h>
# include <string>

# include <libmseed/libmseed.h>

# include "bdata.h"

using namespace std;

/* Structure:
 *
 * Trace list -> Sequence of IDs (DTT or DAT files)
 *   |..Trace -> ID (one DTT or DAT file)
 *      |..... Record -> Batch (reference with 1024 samples)
 *
 */

namespace Zero {
  class Ms {
    public:
      int32_t sequence_number;
      static const double samprate = 250;

      char network[11];
      char station[11];
      char location[11];
      char channel[11];

      /* Main trace list */
      MSTraceList * mstl;

      Ms (const char*, const char*, const char*, const char*);
      ~Ms ();

      void add_bdata (Bdata *);
      void pack_tracelist (const char *);
      static void record_handler (char *, int, void *);

  };
}

