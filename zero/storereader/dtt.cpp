/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * dtt.cpp: Interface to DTT file
 *
 */

# include <iostream>
# include <fstream>
# include <string>

# include "dtt.h"

using namespace std;

namespace Zero {
  Dtt::Dtt (int _id) {
    id = _id;
    cout << "Opening id: " << id << endl;

    read_index ();
    read_batches ();
  }

  void Dtt::read_index () {
    char fname[50];
    sprintf (fname, "%d.ITT", id);

    cout << "Opening: " << fname << endl;

    ifstream itt (fname);

    if (itt.bad () || !itt.is_open ()) {
      cerr << "[e] Could not open index." << endl;
      return;
    }
    itt >> localversion;
    itt >> remoteversion;
    itt >> id;
    itt >> samplescount;
    itt >> batchcount;

    cout << "Local version:  " << localversion << endl;
    cout << "Remote version: " << remoteversion << endl;
    cout << "ID:             " << id << endl;
    cout << "Samples:        " << samplescount << endl;
    cout << "Batches:        " << batchcount << endl;

    itt.close ();
  }

  void Dtt::read_batches () {
    char fname[50];
    sprintf (fname, "%d.DTT", id);
    cout << "Reading batches from: " << fname << endl;

    ifstream dtt (fname);

    if (dtt.bad () || !dtt.is_open ()) {
      cerr << "[e] Could not open data file." << endl;
      return;
    }


    for (int i = 0; i < batchcount; i++) {
      Batch b;
      b.samples = new int[BATCH_LENGTH];

      /* Read reference */
      string ref;
      dtt >> ref;

      //cout << ref << endl;

      (char) dtt.get();
      (char) dtt.get();

      int t = 0;
      int pos = 0;
      string token;
      while (t <= 7 && pos < ref.length()) {
        token = "";
        while (pos < ref.length()) {
          char c = ref[pos];
          if (c == ',') {pos++; break;}
          else token += (char)c;
          pos++;
        }

        switch (t) {
          case 0: break; // R
          case 1: sscanf (token.c_str(), "%u", &(b.length)); break;
          case 2: sscanf (token.c_str(), "%u", &(b.no)); break;
          case 3: sscanf (token.c_str(), "%lu", &(b.ref)); break;
          case 4: sscanf (token.c_str(), "%u", &(b.status)); break;
          case 5: b.latitude  = token;
          case 6: b.longitude = token;
          case 7: sscanf (token.c_str(), "%u", &(b.checksum)); break;
        }

        t++;
      }

      //cout << "Ref: " << b.no << ", " << b.ref << ", status: " << b.status << ", latitude: " << b.latitude << ", longitude: " << b.longitude << ", checksum: " << b.checksum << endl;
      //cout << "Read ref: " << b.no << endl;

      for (int j = 0; j < BATCH_LENGTH; j++) {
        uint32_t s;
        dtt >> s;
        (char) dtt.get();

        if (j < (BATCH_LENGTH -1)) (char) dtt.get(); // don't ask why..

        b.samples[j] = s;
        totalsamples++;
      }

      batches.push_back (b);
    }

    cout << "Read " << totalsamples << " samples total." << endl;
  }

  Dtt::~Dtt () {
    cout << "Closing id: " << id << endl;
  }
}

