/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Main entry point - creates instance of BuoyMaster.
 *
 */

# include "buoy.h"
# include "wirish.h"

using namespace Buoy;


/* Force init to be called *first*, i.e. before static object allocation.
 * Otherwise, statically allocated objects that need libmaple may fail.   */
__attribute__((constructor)) void premain() {
  init();
}


/* Declared in buoy.h */
BuoyMaster * (Buoy::bu);

int main(void) {
  Buoy::bu = new BuoyMaster ();
  Buoy::bu->main ();

  return 0;
}

/* vim: set filetype=arduino :  */

