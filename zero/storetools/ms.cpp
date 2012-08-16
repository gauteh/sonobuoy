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
      msr->starttime  = batch->ref;
      msr->encoding   = DE_STEIM2;

      msr->datasamples = batch->samples;
      msr->numsamples  = batch->length;
      msr->sampletype  = 'i'; // int32
      msr->samplecnt   = batch->length;

      /* Record specific values */

      /* Add MS record to trace list, group by quality, autoheal */
      mstl_addmsr (mstl, msr, 1, 1, batch->ref, SAMPLERATE);

      batch++;
    }


    cout << "done." << endl;
  }

  bool Ms::pack_tracelist (const char *fname) {
    cout << "MS: Packing tracelist.." << endl;

    mstl_printtracelist (mstl, 1, 1, 1);

    bool autofname = (fname == NULL);
    bool sequence_fname = false;
    char *origname;

    if (mstl->numtraces > 1 && fname != NULL) {
      cout << "More than one output file, labeling sequentially." << endl;
      sequence_fname = true;
      origname = new char[strlen(fname) + 1];
      strcpy (origname, fname);
    }

    /* Packing traces */
    for (int i = 0; i < mstl->numtraces; i++) {

      /* Packing trace */
      MSTraceID id = mstl->traces[i];

      /* Generate file name */
      char *thisfname;

      if (autofname) {
        char timestr[50];
        string _fname = id.srcname;
        //_fname += '_';
        _fname += ms_hptime2isotimestr (id.earliest, timestr, 1);
        _fname += ".mseed";
        thisfname = (char*)_fname.c_str ();
      } else if (sequence_fname) {
        string _fname = fname;
        _fname += '_';
        _fname += i;
        thisfname = (char*)_fname.c_str ();
      } else {
        thisfname = (char*)fname;
      }

      ofstream out (thisfname);

      if (!out.is_open () || out.bad ()) {
        cout << "Ms: Error: Could not open file for writing: " << thisfname << endl;
        return false;
      }

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

      mst->samplecnt    = totalsamples;
      mst->numsamples   = totalsamples;
      mst->datasamples  = datasamples;

      /* Packing */
      int psamples, precords;
      precords = mst_pack (mst, &(Ms::record_handler), (void*) &out,
                           4096, DE_STEIM2, 1, &psamples, 1, 2, NULL);

      cout << "MS: => Packed " << psamples << " samples in " << precords << " records to file " << thisfname << "." << endl;

      out.close ();
    }

    return true;
  }

  void Ms::record_handler (char *record, int reclen, void *out) {
    //cout << "Writing " << reclen << " records..";
    ((ofstream*)out)->write (record, reclen);
    //cout << "done." << endl;
  }


  Ms::~Ms () {
    mstl_free (&mstl, 1);
  }
}

