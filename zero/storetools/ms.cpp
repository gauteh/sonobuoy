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
    cout << "MS: Initializing.." << endl;

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
    mstg = mst_initgroup (NULL);
  }

  void Ms::add_bdata (Bdata * b) {
    //cout << "MS: Adding id: " << b->id << "..";

    /* Set up MS record for each batch */
    int samples = 0;
    int records = 0;
    vector<Bdata::Batch>::iterator batch = b->batches.begin ();
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
      msr->encoding   = DE_INT32; // will be redone on packing tracelist

      msr->datasamples = batch->samples_i;
      msr->numsamples  = batch->length;
      msr->sampletype  = 'i'; // int32
      msr->samplecnt   = batch->length;

      /* Record specific values */

      /* Add MS record to trace group, group by quality, autoheal */
      mst_addmsrtogroup (mstg, msr, 1, TIMETOLERANCE, SAMPLERATETOLERANCE);

      samples += msr->numsamples;
      records++;

      batch++;
    }
  }

  bool Ms::pack_group () {
    cout << "MS: Packing tracegroup.." << endl;

    /* Generate file name */
    char srcname[256];
    mst_srcname (mstg->traces, srcname, 0);

    char timestr[50];
    ms_hptime2isotimestr (mstg->traces->starttime, timestr, 0);

    string fname;
    fname += timestr;
    fname += "_";
    fname += srcname;
    fname += ".mseed";

    ofstream out (fname.c_str());

    if (!out.is_open () || out.bad ()) {
      cout << "Ms: Error: Could not open file for writing: " << fname << endl;
      return false;
    }

    /* Packing */
# define DATABLOCK  4096
# define ENCODING   DE_INT32
# define BYTEORDER  1 // Big endian
# define FLUSH      1
# define VERBOSE    1
    int64_t psamples, precords;
    precords = mst_packgroup (mstg, &(Ms::record_handler), (void*) &out,
                              DATABLOCK, ENCODING, BYTEORDER, &psamples,
                              FLUSH, VERBOSE, NULL);

    cout << "MS: => Packed " << psamples << " samples in " << precords << " records to file " << fname << "." << endl;

    out.close ();

    return true;
  }

  void Ms::record_handler (char *record, int reclen, void *out) {
    ((ofstream*)out)->write (record, reclen);
  }


  Ms::~Ms () {
    mst_freegroup (&mstg);
  }
}

