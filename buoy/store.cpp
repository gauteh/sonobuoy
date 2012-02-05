/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-02-05
 *
 * SD store
 *
 */

# include "store.h"
# include "buoy.h"

namespace Buoy {
  Store::Store () {

  }

  void Store::setup (BuoyMaster *b) {
    rf = b->rf;
    ad = b->ad;

  }
}

/* vim: set filetype=arduino :  */

