/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * ms.cpp: Interface to libmseed designed for buoy
 *
 */

# include <iostream>
# include <fstream>
# include <string>

# include <libmseed/libmseed.h>

# include "ms.h"

using namespace std;

namespace Zero {
  Ms::Ms (const char * _network, const char *_station, const char *_location, const char *_channel) {
    cout << "MS initializing.." << endl;

    strcpy (network, _network);
    strcpy (station, _station);
    strcpy (location, _location);
    strcpy (channel, _channel);

    cout << "  Network ..: " << network << endl
         << "  Station ..: " << station << endl
         << "  Location .: " << location << endl
         << "  Channel ..: " << channel << endl;

    /* Ensure big endian headers and data */
    MS_PACKHEADERBYTEORDER(1);
    MS_PACKDATABYTEORDER(1);

    /* Set up trace list */
    mstl = mstl_init (NULL);
  }

  void Ms::add_bdata (Bdata * b) {
    cout << "MS: Adding id: " << b->id << "..";

    /* Set up MS record for each batch */
    vector<Batch>::iterator batch = b->batches.begin ();
    while (batch < b->batches.end ()) {
      MSRecord *msr = msr_init (NULL);

      /* Common values */
      strcpy (msr->network, network);
      strcpy (msr->station, station);
      strcpy (msr->location, location);
      strcpy (msr->channel, channel);

      msr->samprate   = SAMPLERATE;
      msr->byteorder  = 1; // big endian
      msr->starttime   = batch->ref;
      msr->encoding    = DE_INT32;

      msr->datasamples = batch->samples;
      msr->numsamples = batch->length;
      msr->sampletype = 'i'; // int32
      msr->samplecnt  = batch->length;

      /* Add MS record to trace list */
      mstl_addmsr (mstl, msr, 0, 0, batch->ref, SAMPLERATE);
      batch++;
    }


    cout << "done." << endl;
  }

  void Ms::pack_tracelist (const char *fname) {
    cout << "MS: Packing tracelist.." << endl;
    mstl_printtracelist (mstl, 1, 1, 1);

    /* Packing traces */
    for (int i = 0; i < mstl->numtraces; i++) {

      /* Packing trace */
      MSTraceID id = mstl->traces[i];

      string fname = id.srcname;
      fname += ".mseed";

      ofstream out (fname.c_str());

      MSTrace * mst = mst_init (NULL);

      strcpy (mst->network, id.network);
      strcpy (mst->station, id.station);
      strcpy (mst->location, id.location);
      strcpy (mst->channel, id.channel);

      mst->starttime    = id.earliest;
      mst->endtime      = id.latest;
      mst->dataquality  = id.dataquality;

      MSTraceSeg * seg = id.first;

      mst->samprate   = seg->samprate;
      mst->sampletype = seg->sampletype;

      /* Create complete datasamples buffer */
      int totalsamples = 0;
      for (int segno = 0; segno < id.numsegments; segno++) {
        totalsamples += seg->numsamples;
        seg = seg->next;
      }

      int32_t * datasamples = new int32_t[totalsamples];
      uint32_t pos = 0;
      seg = id.first;
      for (int segno = 0; segno < id.numsegments; segno++) {
        memcpy (&(datasamples[pos]), seg->datasamples, sizeof(int32_t) * seg->numsamples);
        pos += seg->numsamples;

        seg = seg->next;
      }

      mst->samplecnt  = totalsamples;
      mst->numsamples = totalsamples;
      mst->datasamples = datasamples;

      /* Packing */
      int psamples, precords;
      precords = mst_pack (mst, &(Ms::record_handler), (void*) &out, 4096, DE_INT32, 1, &psamples, 1, 8, NULL);
      cout << "Packed " << psamples << " samples in " << precords << " records." << endl;

      out.close ();
    }
  }

  void Ms::record_handler (char *record, int reclen, void *out) {
    cout << "Writing " << reclen << " records.." << endl;
    ((ofstream*)out)->write (record, reclen);
  }


  Ms::~Ms () {
    mstl_free (&mstl, 1);
  }
}

