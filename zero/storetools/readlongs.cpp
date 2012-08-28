/* readlongs.cpp: Reads longs from a binary file, output one on each line
 *
 * Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-08-28
 *
 */

# include <iostream>
# include <fstream>
# include <stdint.h>
# include <stdlib.h>

using namespace std;

int main (int argc, char ** argv) {
  if (argc < 2) {
    cerr << "Specify file." << endl;
    exit (1);
  }

  ifstream i (argv[1], ios::binary);

  if (i.bad () || !i.is_open ()) {
    cerr << "Could not open file." << endl;
    exit (1);
  }

  while (!i.eof ()) {
    uint32_t l;
    i.read (reinterpret_cast<char*>(&l), sizeof(l));
    if (!i.eof ())
      cout << l << endl;
  }

  i.close ();

  return 0;
}

