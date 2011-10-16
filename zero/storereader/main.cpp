/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-16
 *
 * Reads index and data files stored on memory card
 *
 */

# define byte char
typedef unsigned long ulong;
typedef unsigned int  uint;
# define ONLY_SPEC

# include "store.h"

# include <iostream>

using namespace std;

int main (int argc, char **argv) {

  cout << "Store reader for Gautebøye (" << VERSION << ")" << endl;
  cout << "Store version: " << STORE_VERSION << endl;

  return 0;
}

