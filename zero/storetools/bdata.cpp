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

  hptime_t MAXTIME () {
    time_t t = time(NULL);
    struct tm *ts = gmtime(&t);
    return ms_time2hptime(ts->tm_year + 1 + 1900, 1, 0, 0, 0, 0);
  }

  hptime_t MINTIME () {
    return ms_time2hptime(2012, 100, 0, 0, 0, 0);
  }

  void Bdata::fix_batch_time () {
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
    fixedtime = false;
    notimefix = false;
    int goodid = -1;

    for (vector<Batch>::iterator b = batches.begin(); b < batches.end (); b++)
    {
      b->fixedtime = false;

      if (b->ref < MAXTIME() && b->ref > MINTIME()) {
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

  Collection::Collection () {
    fixedtime = false;
    notimefix = false;
  }

  void Collection::fix_data_time () {
    /* Try to fix time on data files and batches, based on time on previous
     * or following data files */

    /* Concept:
     *
     * 1. Search for good time, prefer good one with good status
     * 2. Use the closest good time to fix ids with no time
     *
     * Algorithm:
     * 1. Iterate through ids, when good time is found
     * 2. Work backwards to first if missing
     * 3. Work backwards to middle between last good time and this fixing all
     *    ids with missing (even though they have allready been fixed)
     * 4. Use for following bad ids/batches when continuing to iterate
     * 5. When next good time is found update working good time to this
     *
     */

    /* Found good time on */
    uint64_t goodtime = 0;
    uint32_t goodid   = 0;
    uint32_t goodref  = 0;
    bool     hassync  = false;

    for (vector<Bdata>::iterator bd = datas.begin (); bd < datas.end(); bd++) {

      for (vector<Bdata::Batch>::iterator b = bd->batches.begin(); b < bd->batches.end (); b++)
      {

        /* Check if time is good */
        if (b->ref > MINTIME() && b->ref < MAXTIME() && b->status == GOODSTATUS)
        {
          /* Update previous fixes back to half the way to previous good id
           * unless this is the first, then go all the way */
          int halfid = (goodid == 0 ? datas[0].id : (bd->id - goodid) / 2 + goodid);

          /* Update goodtime to this one */
          goodtime = b->ref;
          goodid   = bd->id;
          goodref  = b->no;
          hassync  = true;

          cout << "Found goodtime, I: " << goodid << ", R: " << goodref << endl;
          /* Do the actual updating of previous ones */
          for (vector<Bdata>::iterator p = bd; p->id >= halfid; p--) {

            for (int r = 0; (p->id != bd->id && r < DEFBATCHES) ||
                (p->id == bd->id && r < b->no); r++)
            {
              if (p->batches[r].fixedtime || p->batches[r].notimefix)
              {
                cout << "Fixed time of I: " << p->id << ", R: " << r
                     << " using I: " << goodid << ", R: " << goodref << endl;

                Bdata::Batch *bf = &(p->batches[r]);

                /* Fix time */
                bf->origtime = bf->ref;
                bf->ref = goodtime + ( (p->id - goodid) * 40
                                   + (bf->no - goodref) )
                                   * BATCHLENGTH / SAMPLERATE * 1e6;
                bf->fixedtime   = true;
                bf->notimefix   = false;
                p->fixedtime    = true;
                p->notimefix    = false;

              }
            }
          }

        } else {
          /* Nope, try to fix it if we have a good one already */
          if (goodtime > 0) {
            b->origtime = b->ref;
            b->ref = goodtime + ( (bd->id - goodid) * 40  + (b->no - goodref) )
                              * BATCHLENGTH / SAMPLERATE * 1e6;
            b->fixedtime  = true;
            b->notimefix  = false;
            bd->fixedtime = true;
            bd->notimefix = false;
          } else {
            b->notimefix  = true;
            bd->notimefix = true;
          }
        }
      }


    }
  }
}

