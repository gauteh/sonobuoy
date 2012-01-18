/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# include "wirish.h"
# include "buoy.h"
# include "ads1282.h"

namespace Buoy {

  ADS1282::ADS1282 () {
    batchready = false;
    lastvalue  = 0;

    return;
  }

  void ADS1282::setup () {
    /* Setup AD and get ready for data */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Setting up ADS1282..");
# endif

    /* Configure AD */
    configure ();
  }

  void ADS1282::configure () {
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Configuring ADS1282..");
# endif
    /* Not implemented */
  }

  void ADS1282::read_control_register () {
    /* Not implemented */
  }

  void ADS1282::acquire () {
    /* Not implemented */
  }
}

/* vim: set filetype=arduino :  */

