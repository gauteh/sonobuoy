/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-20
 *
 * dttfix.cpp: Reads a range of IDs, try to fix them and outputs new.
 *
 * Files are expected to be in current directory. Output will be written to
 * files in current directory.
 *
 */

# include <stdint.h>
# include <iostream>
# include <fstream>
# include <vector>

# include "bdata.h"
# include "dtt.h"

# define VERSION GIT_DESC

using namespace std;

namespace Zero {
  namespace DttFix {

    void usage ();

    vector<int> ids;

    int main (int argc, char ** argv) {
      cout << "DttFix ( rev " << VERSION << " )" << endl;
      cout << "Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-08-15" << endl << endl;

      //cout << "Parsing id list.."; // {{{
      if (argc < 2) {
        cerr << "[E] No ids specified." << endl;
        usage ();
        return 1;
      }

      for (int i = 1; i < argc; i++) {
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

      /* Data collection */
      Collection c;

      /* Work through ids */
      vector<int>::iterator id = ids.begin ();
      while (id < ids.end ()) {

        /* Load DTT */
        Dtt dtt (*id); // fixes internal batch time if possible

        if (dtt.ready) {
          c.datas.push_back (*(dtt.bdata));
        } else {
          cout << "Error with: " << *id << ", skipping.." << endl;
        }

        id++;
      }

      /* Fixing time */
      cout << "Fixing time.." << endl;
      c.fix_data_time ();

      /* Write back */
      cout << "Write back files.." << endl;
      for (vector<Bdata>::iterator bd = c.datas.begin (); bd < c.datas.end(); bd++) {
        Dtt dtt (&(*bd));

        char fname_i[50];
        char fname_d[50];
        sprintf (fname_i, "%d.ITT.NEW", bd->id);
        sprintf (fname_d, "%d.DTT.NEW", bd->id);

        dtt.write (fname_i, fname_d);
      }

      cout << "Finished." << endl;

      return 0;
    }

    void usage () {
      cout << endl << "Usage: dttfix sequence.." << endl;
      cout << "Specify ids as space separated sequence of single ids or range" << endl
           << "with a range of the format STARTID-ENDID (no spaces, start and" << endl
           << "end separated with a -)." << endl;
      cout << endl;
      cout << "Output will be written to ID.DTT.NEW and ID.ITT.NEW." << endl;
    }
  }
}

int main (int argc, char ** argv) {
  return Zero::DttFix::main (argc, argv);
}

