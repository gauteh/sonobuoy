/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * dtttomseed.cpp: Converts a range of DTT's to a miniSEED store
 *
 * Requires: libmseed (developed for v2.5.1)
 */

# include <iostream>

# include "dtt.h"
# include "libmseed.h"

# define VERSION GIT_DESC

using namespace std;

namespace Zero {
  namespace DttToMseed {

    void usage ();

    int main (int argc, char ** argv) {
      cerr << "DttToMseed ( rev " << VERSION << " )" << endl;
      cerr << "Author: Gaute Hope <eg@gaute.vetsj.com>, 2012-08-15" << endl;

      if (argc < 2) {
          usage ();
          return 1;
      }


      return 0;
    }

    void usage () {
      cerr << endl << "Usage:" << endl;
      cerr << "Specify ids as space separated sequence of single ids or range" << endl
           << "with a range of the format STARTID-ENDID (no spaces, start and" << endl
           << "end separated with a -)." << endl;

    }
  }
}

int main (int argc, char ** argv) {
  return Zero::DttToMseed::main (argc, argv);
}

