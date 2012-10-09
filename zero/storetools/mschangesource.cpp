/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-16
 *
 * mschangesource.cpp: Changes name of source on input miniseed files
 * and repacks them in new files with the corresponding source name.
 *
 * Update defines at beginning of file to configure source, or set
 * to NULL to use existing.
 *
 */

# include <stdint.h>
# include <iostream>
# include <fstream>
# include <vector>
# include <string.h>
# include <string>

# include <libmseed/libmseed.h>

using namespace std;

/* Configuration */
# define NETWORK  "GB"
# define STATION  "BUO2"
# define LOCATION "G2"
# define CHANNEL  "BNR"

# define VERSION GIT_DESC

/* Macros for stringifying defines */
# define STRINGIFY_I(s) #s
# define STRINGIFY(s)   STRINGIFY_I(s)

namespace Zero {
  namespace MsChangeSource {
    void usage ();
    void record_handler (char *, int, void *);

    vector<string> files;

    int main (int argc, char **argv) {
      cout << "MsChangeSource ( rev " << VERSION << " )" << endl;
      cout << "Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-08-16" << endl << endl;


      //cout << "Parsing id list.."; // {{{
      if (argc < 2) {
        cerr << "[E] No files specified." << endl;
        usage ();
        return 1;
      }

      for (int i = 1; i < argc; i++) {
        string s = argv[i];
        files.push_back (s);
      }
      //cout << "done." << endl;
      // }}}

      for (vector<string>::iterator f = files.begin(); f < files.end(); f++) {
        cout << "Updating source parameters on file: " << *f << endl;

        MSTraceGroup *mstg = NULL;
        int retcode;

        retcode = ms_readtraces (&mstg, (char*)f->c_str(), 0, -1.0, -1.0, 1, 0, 1, 1);

        if (retcode != MS_NOERROR) {
          cout << "Error reading file: " << *f << ", skipping." << endl;
          mst_freegroup (&mstg);
          continue;
        }

        hptime_t starttime = mstg->traces[0].starttime; // start time of traces
        char srcname[50];                               // for generating file name, use source of first trace

        /* Update source on all traces */
        for (int i = 0; i < mstg->numtraces; i++) {
          if (NETWORK != NULL) {
            strcpy (mstg->traces[i].network, NETWORK);
          }

          if (STATION != NULL) {
            strcpy (mstg->traces[i].station, STATION);
          }

          if (LOCATION != NULL) {
            strcpy (mstg->traces[i].location, LOCATION);
          }

          if (CHANNEL != NULL) {
            strcpy (mstg->traces[i].channel, CHANNEL);
          }
        }

        mst_srcname (mstg->traces, srcname, 0); // get updated srcname for use in file name

        mst_printtracelist (mstg, 1, 1, 1);

        /* Write out new file */
        string thisfname;
        char timestr[50];
        thisfname   = srcname;
        thisfname += '_';
        thisfname += ms_hptime2isotimestr (starttime, timestr, 0);
        thisfname += ".mseed";

        ofstream out (thisfname.c_str());

        if (!out.is_open () || out.bad ()) {
          cout << "Error: Could not open file for writing: " << thisfname << ", skipping." << endl;
          continue ;
        }

        cout << "Packing.." << endl;

        int64_t precords, psamples;

# define DATABLOCK  4096
# define ENCODING   DE_STEIM1
# define BYTEORDER  1 // Big endian
# define FLUSH      1
# define VERBOSE    1
        precords = mst_packgroup (mstg,
                                  &record_handler,
                                  (void*) (&out),
                                  DATABLOCK,
                                  ENCODING,
                                  BYTEORDER,
                                  &psamples,
                                  FLUSH,
                                  VERBOSE,
                                  NULL);
        cout << "Packed " << psamples << " samples in " << precords << " records to file " << thisfname << endl;

        out.close ();
      }

      return 0;
    }

    void record_handler (char *record, int reclen, void *out) {
      ((ofstream*)out)->write (record, reclen);
    }

    void usage () { // {{{
      cout  << endl;
      cout  << "Usage: MsChangeSource file1 file2 ... filen" << endl
            << endl
            << "Each file will be repacked with its source rewritten, the source " << endl
            << "is specified in the source code of this file and is currently: " << endl
            << "Network ..: " << STRINGIFY(NETWORK) << endl
            << "Station ..: " << STRINGIFY(STATION) << endl
            << "Location .: " << STRINGIFY(LOCATION) << endl
            << "Channel ..: " << STRINGIFY(CHANNEL) << endl;
    } // }}}
  }
}

int main (int argc, char ** argv) {
  return Zero::MsChangeSource::main (argc, argv);
}

