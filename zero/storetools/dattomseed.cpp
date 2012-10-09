/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-22
 *
 * dattomseed.cpp: Converts a range of DAT's to a hourly miniSEED stores
 *
 * Requires: libmseed (developed for v2.7)
 *
 * All the ids supplied as an argument will be packed into one contiuous trace
 * list, if you want more than one trace list make numerous invocations of
 * this program with each group that should be packed into each trace list.
 *
 * Files are expected to be in current directory. Output will be written to
 * files in current directory in hourly intervals.
 *
 * TODO:
 *  - Output instrument response header?
 *  - Data quality?
 *  - Enhancement: Configuration file
 */

# include <stdint.h>
# include <iostream>
# include <fstream>
# include <vector>

# include "bdata.h"
# include "dat.h"
# include "ms.h"

# define VERSION GIT_DESC

/* Configuration */
# define NETWORK  "GB"
# define LOCATION "G2"
# define CHANNEL  "BNR"

using namespace std;

namespace Zero {
  namespace DatToMseed {

    void usage ();

    vector<int> ids;

    int main (int argc, char ** argv) {
      cout << "DatToMseed ( rev " << VERSION << " )" << endl;
      cout << "Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-08-22" << endl << endl;

      //cout << "Parsing id list.."; // {{{
      if (argc < 2) {
        cerr << "[E] No ids specified." << endl;
        usage ();
        return 1;
      }

      string station = argv[1];

      for (int i = 2; i < argc; i++) {
        int start, end;
        int n = sscanf (argv[i], "%d-%d", &start, &end);
        if (n == 2) {
          for (; start <= end; start++) {
            ids.push_back (start);
          }
        } else {
          int id;
          n = sscanf (argv[i], "%d", &id);
          if (n == 1) {
            ids.push_back (id);
          } else {
            cerr << "[e] Invalid argument: " << argv[i] << endl;
            return 1;
          }
        }
      }
      //cout << "done." << endl;
      // }}}

      /* Set up miniSeed record (template) and trace list */
      Ms ms (NETWORK, station.c_str(), LOCATION, CHANNEL);

      bool err = false;

      /* Work through ids */
      vector<int>::iterator id = ids.begin ();
      while (id < ids.end ()) {

        /* Load DAT */
        cout << "Reading id: " << *id << "..";

        Dat dat (*id);

        dat.bdata->fix_batch_time ();
        dat.bdata->assess_dataquality ();

        /* Add to MS and write */
        if (dat.ready) {
          ms.add_bdata (dat.bdata);

          cout << "done, read and packed " << dat.bdata->totalsamples << " samples." << endl;

        } else {
          cerr << ": Error with: " << *id << ", skipping.." << endl;
          err = true;
        }

        id++;
      }

      /* Pack traces */
      if ( !ms.pack_group () ) {
        cout << "Failed, see above errors." << endl;
      }

      if (err) {
        cerr << "There were errors, please see above for details." << endl;
        return 1;
      }

      return 0;
    }

    void usage () {
      cout << endl << "Usage: dattomseed STATION sequence.." << endl;
      cout << "Specify ids as space separated sequence of single ids or range" << endl
           << "with a range of the format STARTID-ENDID (no spaces, start and" << endl
           << "end separated with a -)." << endl;

    }
  }
}

int main (int argc, char ** argv) {
  return Zero::DatToMseed::main (argc, argv);
}

