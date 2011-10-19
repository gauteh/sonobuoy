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
# include <getopt.h>

using namespace std;

# define ONLY_SPEC
typedef char byte;
# include "store.h"

Index open_index (string);
void  print_index (Index);
void  usage (string);

int main (int argc, char **argv) {
  string self = argv[0];

  cerr << "Store reader for Gautebøye ( rev " << VERSION << ")" << endl;
  cerr << endl;
  cerr << "Store version ......: " << STORE_VERSION << endl;
  cerr << "Max samples ........: " << MAX_SAMPLES_PER_FILE << endl;
  cerr << "Max references .....: " << MAX_REFERENCES << endl;
  cerr << "Standard file size .: " << SD_DATA_FILE_SIZE << endl;
  cerr << endl;

  /* Option parsing */
  if (argc < 2) {
    cerr << "[ERROR] You have to specify some options, see usage." << endl;
    usage (self);
    exit (1);
  }

  int opt;
  bool opt_only_index = false;

# define DEFAULT_FORMAT "[%09lu] %07lu\n"
  string format = DEFAULT_FORMAT;

  while ((opt = getopt(argc, argv, "iuhHf:")) != -1) {
    switch (opt)
    {
      case 'h':
      case 'H':
      case 'u':
        usage (self);
        exit (0);
        break;
      case 'i':
        opt_only_index = true;
        break;
      case 'f':
        format = optarg;
        break;
      case '?':
        usage (self);
        exit (1);
    }
  }

  if (optind >= argc) {
    cerr << "[ERROR] You must specify an id." << endl;
    exit (1);
  }


  string indexfn (argv[optind]);
  string datafn (indexfn);
  datafn.replace (indexfn.rfind (".IND"), 4, ".DAT");

  Index i = open_index (indexfn);
  print_index (i);

  if (i.samples > 0 && !opt_only_index) {
    cerr << "=> Reading data..";
    /* Opening DATA */
    ifstream fd (datafn.c_str (), ios::binary);
    if (!fd.is_open ()) {
      cerr << endl << "[ERROR] Could not open data file: " << datafn << endl;
      exit (1);
    }

    int ref = 0;
    int sam = 0;
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

      //cout << "[" << timestamp << "] " << dec << uppercase << right << setw(8) << s << nouppercase << dec << endl;
      printf (format.c_str (), timestamp, s);

      sam++;

      if (s == 0) {
        cerr << "=> [ERROR] Sample == 0" << endl;
        break;
      }
    }

    cerr << "=> Read " << sam << " samples (of " << i.samples << " expected)." << endl;
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

void usage (string argv) {
  cerr << endl << "Usage: " << argv << " [-u|-h] [-f format] [-i] INDEX_FILE" << endl;
  cerr << endl;
  cerr << " -u or -h      Print this help text." << endl;
  cerr << " -i            Only print index." << endl;
  cerr << endl;
  cerr << " -f format     Format specifies output format of data file, " \
          "follows" << endl;
  cerr << "               printf syntax, where the first argument will be " \
          "the" << endl;
  cerr << "               the time stamp and second the sample. Both are of" \
       << endl;
  cerr << "               the type uint32_t." << endl;
  cerr << "               Default: " << DEFAULT_FORMAT; // Has newline at end
  cerr << endl;
  cerr << "  A datafile (.DAT) with the same name as the INDEX_FILE is " \
          "expected." << endl;
}

