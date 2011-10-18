/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-16
 *
 * Reads index and data files stored on memory card
 *
 */

# include <stdint.h>
# include <fstream>
# include <iostream>
# include <stdlib.h>

using namespace std;

# define ONLY_SPEC
typedef char byte;
# include "store.h"

Index open_index (string);
void  print_index (Index);

int main (int argc, char **argv) {

  cerr << "Store reader for Gautebøye ( rev " << VERSION << ")" << endl;
  cerr << "Store version: " << STORE_VERSION << endl << endl;

  string indexfn = "1.IND";
  string datafn  = "1.DAT";

  Index i = open_index (indexfn);
  print_index (i);

  return 0;
}

Index open_index (string fn) {
  cerr << "Opening index " << fn << "..";

  Index i;

  ifstream fi (fn.c_str (), ios::binary);

  if (!fi.is_open ()) {
    cerr << endl << "[ERROR] Could not open index, exiting..";
    exit(1);
  }

  /* Reading index, member by member to avoid struct padding issues */
  fi.read (reinterpret_cast<char*>(&i.version), sizeof(i.version));
  fi.read (reinterpret_cast<char*>(&i.id), sizeof(i.id));
  fi.read (reinterpret_cast<char*>(&i.sample_l), sizeof(i.sample_l));
  fi.read (reinterpret_cast<char*>(&i.timestamp_l), sizeof(i.timestamp_l));
  fi.read (reinterpret_cast<char*>(&i.samples), sizeof(i.samples));
  fi.read (reinterpret_cast<char*>(&i.nrefs), sizeof(i.nrefs));
  fi.read (reinterpret_cast<char*>(&i.refs), i.nrefs * sizeof(uint32_t));

  cerr << "done." << endl;
  return i;
}

void print_index (Index i) {
  cerr << "=> Index:             " << i.id << endl;
  cerr << "=> Version:           " << i.version << endl;
  cerr << "=> Sample length:     " << i.sample_l << endl;
  cerr << "=> Timestamp length:  " << i.timestamp_l << endl;
  cerr << "=> Samples:           " << i.samples << endl;
  cerr << "=> References:        " << i.nrefs << endl;
}

