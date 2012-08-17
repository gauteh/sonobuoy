/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-16
 *
 * bdata.cpp: Data structure for id with batches as well as methods for
 *            fixing and sanitizing data.
 *
 * Warning: This program assumes that in32_t are represented with two's
 *          complement uint32_t.
 *
 */

# include <stdint.h>
# include <time.h>
# include <iostream>
# include <vector>

# include <libmseed/libmseed.h>

# include "bdata.h"

namespace Zero {
  void Bdata::check_checksums () {
    checksum_passed = false;

    for (vector<Batch>::iterator b = batches.begin(); b < batches.end (); b++) {
      uint32_t cs = 0;
      b->checksum_pass = false;

      for (int i = 0; i < b->length; i++) {
        cs ^= b->samples_u[i];
      }

      b->checksum_pass = (b->checksum == cs);

      if (!b->checksum_pass) {
        cout << "BData, ID: " << id << ", batch: " << b->no << ": Checksum fail." << endl;
      }
      checksum_passed &= b->checksum_pass;
    }
  }

  void Bdata::populate_int32_samples () {
    /* Convert uint32_t to int32_t */
    for (vector<Batch>::iterator b = batches.begin(); b < batches.end (); b++) {
      b->samples_i  = new int32_t[b->length];
      b->hasclipped = false;

      for (int i = 0; i < b->length; i++) {
        /* The sample is stored as a two's complement 32 bit uint,
         * the last bit indicates whether the full scale of the AD
         * has been exceeded. At upper value a set last bit means overflow,
         * at lower value a unset last bit means underflow.
         */

        uint32_t s = b->samples_u[i];
        int32_t  ss;

        b->hasclipped |= s & 0x1;
        s   &= 0xfffffffe; // Mask out to avoid confusion with two's comp
        ss   = s;          // Assume arch. stores int32_t's as two's comp
        b->samples_i[i] = ss;
      }
    }
  }

  void Bdata::fix_time () {
    /* Search for bad jumps in time and realign to previous sane reference
     *
     * Let GPS sync and time drift stay:
     *  - Small jumps forward on each batch
     *  - Time recovery and jumps of seconds/minutes afterwards
     *
     * Weed out:
     *  - Overflows
     *  - Bad dates: Large as well as far into the future
     *
     * The idea is that on the scale that each ID spans the drift is
     * acceptable and simply realigning to previous sane reference should
     * be alright.
     *
     * Notes on store versions:
     * - Because of E_SDLAG, frequent checksum fails in periods below version 6.
     * - HAS_SYNC and HAS_SYNC_REFERENCE are unusable below store version 7.
     * - Time could not be updated backwards below version 7: will result in
     *   a date in the future to not be reset back.
     */

    /* Any times out of this range will be ignored and time will be
     * extrapolated from previous sane reference */
    time_t t      = time (NULL);
    struct tm *ts = gmtime (&t);

# define MAXTIME ms_time2hptime(ts->tm_year + 1 + 1900, 1, 0, 0, 0, 0)
# define MINTIME ms_time2hptime(2012, 100, 0, 0, 0, 0)

    fixedtime = false;
    notimefix = false;
    int goodid = -1;

    for (vector<Batch>::iterator b = batches.begin(); b < batches.end (); b++) {
      b->fixedtime = false;

      if (b->ref < MAXTIME && b->ref > MINTIME) {
        /* ref is good */

        /* First good: Go back and fix preceeding ids */
        if (b->no > 0 && goodid == -1) {
          cout << "Bdata: Fixing time on batches 0 to " << (b->no -1) << ".." << endl;
          for (int i = 0; i < (b->no-1); i++) {
            batches[i].fixedtime = true;
            batches[i].origtime  = batches[i].ref;

            batches[i].ref = b->ref - ( (b->no - i) * BATCHLENGTH / SAMPLERATE * 1e6);
            fixedtime = true;
          }
        }

        goodid = b->no;

      } else {
        /* ref is bad */
        if (goodid > -1) {
          /* There exists a good reference, use it */
          b->fixedtime = true;
          b->origtime  = b->ref;
          b->ref = batches[goodid].ref + ( (b->no - goodid) * BATCHLENGTH / SAMPLERATE * 1e6);
          fixedtime = true;
          cout << "Bdata: Fixing time on batch " << b->no << ".." << endl;
        }
      }
    }

    /* No good refs for id */
    if (goodid == -1) {
      cout << "Bdata: No good refs for ID, time could not be fixed." << endl;
      notimefix = true;
    }
  }

  void Bdata::assess_dataquality () {

  }
}

