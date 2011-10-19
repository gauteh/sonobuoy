/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-16
 *
 * Reads index and data files stored on memory card
 *
 */

# include <stdint.h>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <stdlib.h>

using namespace std;

# define ONLY_SPEC
typedef char byte;
# include "store.h"

Index open_index (string);
void  print_index (Index);
void  usage (char **);

int main (int argc, char **argv) {

  cerr << "Store reader for Gautebøye ( rev " << VERSION << ")" << endl;
  cerr << endl;
  cerr << "Store version:  " << STORE_VERSION << endl;
  cerr << "Max samples:    " << MAX_SAMPLES_PER_FILE << endl;
  cerr << "Max references: " << MAX_REFERENCES << endl;
  cerr << "Data file size: " << SD_DATA_FILE_SIZE << endl;
  cerr << endl;

  if (argc < 2) {
    cerr << "[ERROR] No ID specified." << endl;
    usage (argv);
    exit (1);
  }

  string indexfn (argv[1]);
  string datafn = indexfn + ".DAT";
  indexfn += ".IND";

  Index i = open_index (indexfn);
  print_index (i);

  cerr << "Opening data..";
  /* Opening DATA */
  ifstream fd (datafn.c_str (), ios::binary);
  if (!fd.is_open ()) {
    cerr << endl << "[ERROR] Could not open data file." << endl;
    exit (1);
  }

  int ref = 0;
  while (!fd.eof ())
  {
    if (ref < i.nrefs) {
      if (fd.tellg() == i.refs[ref]) {
        /* On index, reading.. */
        bool failref = false;
        uint32_t refid;
        uint32_t ref;
        uint32_t refstatus;

        cerr << "=> On reference: " << ref << "..";
        for (int k = 0; k < (3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); k++) {
          int r = fd.get ();
          if (r != 0) {
            failref = true;
          }
        }

        fd.read (reinterpret_cast<char*>(&refid), sizeof(uint32_t));
        fd.read (reinterpret_cast<char*>(&ref), sizeof(uint32_t));
        fd.read (reinterpret_cast<char*>(&refstatus), sizeof(uint32_t));

        for (int k = 0; k < (3 * (SAMPLE_LENGTH + TIMESTAMP_LENGTH)); k++) {
          int r = fd.get ();
          if (r != 0) {
            failref = true;
          }
        }

        cerr << "=> Reference id: " << refid << endl;
        cerr << "=> Reference   : " << ref << endl;
        cerr << "=> Status      : " << refstatus << endl;

        ref++;
      }
    }

    /* On timestamp / sample pair */
    uint32_t timestamp;
    uint32_t tt;
    sample ss;
    uint32_t s = 0;

    fd.read (reinterpret_cast<char*>(&tt), sizeof(tt));
    fd.read (reinterpret_cast<char*>(&ss), sizeof(sample));

    s =  ((uint8_t)(ss[0]) << 16);
    s += ((uint8_t)(ss[1]) << 8);
    s +=  (uint8_t)(ss[2]);

    // TODO: Endianness probs?
    //timestamp = __builtin_bswap32 (tt);
    timestamp = tt;

    cout << "[" << timestamp << "] " << hex << uppercase << right << setw(8) << s << nouppercase << dec << endl;

    if (s == 0) break;
  }


  return 0;
}

Index open_index (string fn) {
  cerr << "Opening index " << fn << "..";

  Index i;

  ifstream fi (fn.c_str (), ios::binary);

  if (!fi.is_open ()) {
    cerr << endl << "[ERROR] Could not open index." << endl;
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
  for (int j = 0; j < i.nrefs; j++) {
    cerr << "=>        [" << j << "]: " << (i.refs[j]) << endl;
  }
}

void usage (char **argv) {
  cerr << endl << "Usage: " << argv[0] << " id" << endl << endl;
}

