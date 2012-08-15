/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-08-15
 *
 * ms.cpp: Interface to libmseed designed for buoy
 *
 */

# include <iostream>
# include <fstream>

# include <libmseed/libmseed.h>

# include "ms.h"

using namespace std;

namespace Zero {
  Ms::Ms () {
    cout << "Setting up miniSeed trace list.." << flush << endl;
  }

  Ms::~Ms () {
    cout << "Closing miniSeed" << flush << endl;
  }
}

