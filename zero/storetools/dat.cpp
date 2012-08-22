/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-22
 *
 * dat.cpp: Interface to DAT file
 *
 */

# include <iostream>
# include <fstream>
# include <string>

# include "dat.h"
# include "bdata.h"

using namespace std;

namespace Zero {


  Dat::Dat (int _id) {
    bdata = new Bdata;
    bdata->id = _id;
    cout << "Opening id: " << bdata->id << "..";

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

    cout << "done, read: " << bdata->batches.size () << " batches with: " << bdata->totalsamples << " samples total." << endl;
    ready = true;
  }

  Dat::Dat (Bdata *b) {
    bdata = b;
    ready = true;
    cout << "Initialized id: " << bdata->id << "..";
    cout << "done, loaded: " << bdata->batches.size () << " batches with: " << bdata->totalsamples << " samples total." << endl;
  }

  bool Dat::read_index () {
    char fname[50];
    sprintf (fname, "%d.IND", bdata->id);

    //cout << "Opening: " << fname << endl;

    ifstream ind (fname, ios::binary);
    if (ind.bad () || !ind.is_open ()) {
      cerr << "Could not open index file." << endl;
      return false;
    }


    Index i;

    /* Reading index, member by member to avoid struct padding issues */
    ind.read (reinterpret_cast<char*>(&i.version), sizeof(i.version));
    ind.read (reinterpret_cast<char*>(&i.id), sizeof(i.id));
    ind.read (reinterpret_cast<char*>(&i.sample_l), sizeof(i.sample_l));
    ind.read (reinterpret_cast<char*>(&i.samples), sizeof(i.samples));
    ind.read (reinterpret_cast<char*>(&i.samples_per_reference), sizeof(i.samples_per_reference));
    ind.read (reinterpret_cast<char*>(&i.nrefs), sizeof(i.nrefs));

    if (i.version > 8) {
      ind.read (reinterpret_cast<char*>(&i.e_sdlag), sizeof(i.e_sdlag));
    } else {
      i.e_sdlag = false;
    }

    bdata->source         = 1;
    bdata->localversion   = 0;
    bdata->remoteversion  = i.version;
    bdata->totalsamples   = i.samples;
    bdata->batchcount     = i.nrefs;
    bdata->hasfull        = true;
    bdata->e_sdlag        = i.e_sdlag;

    ind.close ();
    return true;
  }

  bool Dat::read_batches () {
    char fname[50];
    sprintf (fname, "%d.DAT", bdata->id);
    //cout << "Reading batches from: " << fname << endl;

    ifstream dat (fname, ios::binary);

    if (dat.bad () || !dat.is_open ()) {
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

      /* Parse reference */

      //cout << "Ref: " << b.no << ", " << b.ref << ", status: " << b.status << ", latitude: " << b.latitude << ", longitude: " << b.longitude << ", checksum: " << b.checksum << endl;
      //cout << "Read ref: " << b.no << endl;

      if (b.no != i) {
        cout << "Refrence number does not match position in sequence. Cancelling." << endl;
        return false;
      }

      /* Read samples */
      for (int j = 0; j < BATCH_LENGTH; j++) {
      }

      bdata->batches.push_back (b);
    }

    //cout << "Read " << totalsamples << " samples total." << endl;
    return true;
  }

  /*
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
    out << bdata->samplescount << endl;
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
  */

  Dat::~Dat () {
  }
}

