

# include "buoy.h"
using namespace std;
using namespace Buoy;

/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include "wirish.h"


// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor)) void premain() {
    init();
}

int main(void) {
  BuoyMaster b;
  b.main ();

  return 0;
}

/* vim: set filetype=arduino :  */

