/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-16
 *
 * bdata.cpp: Data structure for id with batches as well as methods for
 *            fixing and sanitizing data.
 *
 */

# include <stdint.h>
# include <iostream>
# include <vector>

# include <libmseed/libmseed.h>

# include "bdata.h"

namespace Zero {
  void Bdata::check_checksums () {
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
    }
  }

  void Bdata::populate_int32_samples () {
    /* Convert uint32_t to int32_t */
    for (vector<Batch>::iterator b = batches.begin(); b < batches.end (); b++) {
      b->samples_i  = new int32_t[b->length];
      b->hasclipped = false;

      for (int i = 0; i < b->length; i++) {
        uint32_t s = b->samples_u[i];
        int32_t  ss;

        b->hasclipped |= s & 0x1;
        s   &= 0xfffffffe; // Mask out to avoid confusion with two's comp
        ss   = s;          // Assume arch. stores int32_t's as two's comp
        ss >>= 1;          // shift down to 31 bits (clear FS clip bit)
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
     * - Because of SD_LAG, frequent checksum fails in periods below version 6.
     * - HAS_SYNC and HAS_SYNC_REFERENCE are unusable below store version 7.
     * - Time could not be updated backwards below version 7: will result in
     *   a date in the future to not be reset back.
     */

    /* Any times out of this range will be ignored and time will be
     * extrapolated from previous sane reference */
# define MAXTIME ms_time2hptime(2013, 1, 0, 0, 0, 0)
# define MINTIME ms_time2hptime(2012, 100, 0, 0, 0, 0)

    for (vector<Batch>::iterator b = batches.begin(); b < batches.end (); b++) {
      b->fixedtime = false;

    }
  }

  void Bdata::assess_dataquality () {

  }
}

