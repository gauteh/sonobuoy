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
# include "bdata.h"

using namespace std;

namespace Zero {
  Dtt::Dtt (int _id) {
    bdata = new Bdata;
    bdata->id = _id;
    //cout << "Opening id: " << bdata->id << "..";

    /* Load index and samples */
    if (!read_index ()) {
      ready = false;
      return;
    }
    if (!read_batches ()) {
      ready = false;
      return;
    }

    /* Check and prepare samples */
    bdata->check_checksums ();
    bdata->populate_int32_samples ();

    //cout << "done, read: " << bdata->batches.size () << " batches with: " << bdata->totalsamples << " samples total." << endl;
    ready = true;
  }

  Dtt::Dtt (Bdata *b) {
    bdata = b;
    ready = true;
    //cout << "Initialized id: " << bdata->id << "..";
    //cout << "done, loaded: " << bdata->batches.size () << " batches with: " << bdata->totalsamples << " samples total." << endl;
  }

  bool Dtt::read_index () {
    char fname[50];
    sprintf (fname, "%d.ITT", bdata->id);

    //cout << "Opening: " << fname << endl;

    ifstream itt (fname);

    if (itt.bad () || !itt.is_open ()) {
      cerr << "Could not open index." << endl;
      return false;
    }

    bdata->source = 0;
    itt >> bdata->localversion;
    itt >> bdata->remoteversion;
    itt >> bdata->id;
    itt >> bdata->totalsamples;
    itt >> bdata->batchcount;

    bdata->hasfull = false;
    bdata->e_sdlag = false;

    string s;
    itt >> s;
    if (s == "True") bdata->hasfull = true;
    itt.get (); // skip newline

    if (bdata->localversion >= 3) {
      itt >> s;
      if (s == "True") bdata->e_sdlag = true;
    }

    /*
    cout << "Local version:  " << localversion << endl;
    cout << "Remote version: " << remoteversion << endl;
    cout << "ID:             " << id << endl;
    cout << "Samples:        " << samplescount << endl;
    cout << "Batches:        " << batchcount << endl;
    */

    itt.close ();
    return true;
  }

  bool Dtt::read_batches () {
    char fname[50];
    sprintf (fname, "%d.DTT", bdata->id);
    //cout << "Reading batches from: " << fname << endl;

    ifstream dtt (fname);

    if (dtt.bad () || !dtt.is_open ()) {
      cerr << "Could not open data file." << endl;
      return false;
    }


    /* Read batches */
    for (int i = 0; i < bdata->batchcount; i++) {
      Bdata::Batch b;

      b.hasclipped    = false;
      b.checksum_pass = false;
      b.fixedtime     = false;
      b.notimefix     = false;
      b.origtime      = 0;

      b.samples_u = new uint32_t[BATCH_LENGTH];

      /* Read reference */
      string ref;
      dtt >> ref;
      (char) dtt.get(); // skip newline

      /* Parse reference */
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

      if (b.no != i) {
        cout << "Refrence number does not match position in sequence. Cancelling." << endl;
        return false;
      }

      /* Read samples */
      for (int j = 0; j < BATCH_LENGTH; j++) {
        uint32_t s;
        dtt >> s;
        (char) dtt.get(); // skip newline

        b.samples_u[j] = s;
      }

      bdata->batches.push_back (b);
    }

    //cout << "Read " << totalsamples << " samples total." << endl;
    return true;
  }

  void Dtt::write (const char *fname_i, const char *fname_d) {
    cout << "Writing " << fname_d << " (and " << fname_i << ").." << endl;
    write_index (fname_i);
    write_batches (fname_d);
  }
  void Dtt::write_index (const char * fname) {
    ofstream out (fname);
    if (!out.is_open () || out.bad ()) {
      cout << "Dtt: Error: Could not open file for writing: " << fname << endl;
      return;
    }

    out << bdata->localversion << endl;
    out << bdata->remoteversion << endl;
    out << bdata->id << endl;
    out << bdata->totalsamples << endl;
    out << bdata->batchcount << endl;
    out << (bdata->hasfull ? "True" : "False") << endl;  // has full
    out << (bdata->e_sdlag ? "True" : "False") << endl; // sd lag?

    for (vector<Bdata::Batch>::iterator b = bdata->batches.begin (); b < bdata->batches.end(); b++) {
      out << b->no << "," << b->ref << "," << b->status << ","
          << b->latitude << "," << b->longitude << "," << b->checksum << ","
          << (b->length * b->no + b->no) << ",0" << endl;
    }

    out.close ();
  }

  void Dtt::write_batches (const char * fname) {
    ofstream out (fname);
    if (!out.is_open () || out.bad ()) {
      cout << "Dtt: Error: Could not open file for writing: " << fname << endl;
      return;
    }

    for (vector<Bdata::Batch>::iterator b = bdata->batches.begin (); b < bdata->batches.end(); b++) {
      out << "R," << b->length << "," << b->no << "," << b->ref << ","
          << b->status << "," << b->latitude << "," << b->longitude << ","
          << b->checksum << endl;
      for (int i = 0; i < b->length; i++) {
        out << b->samples_u[i] << endl;
      }
    }


    out.close ();
  }

  Dtt::~Dtt () {
  }
}

