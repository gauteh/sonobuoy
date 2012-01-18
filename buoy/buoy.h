/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# pragma once

# include "ads1282.h"

/* Version and settings */

/* Define to have debug messages sent to USB serial */
# define DIRECT_SERIAL 1

namespace Buoy {
  class BuoyMaster {
    public:

      ADS1282 ad;


      BuoyMaster ();
      void main ();

    private:
      void setup ();
  };
}

/* vim: set filetype=arduino :  */

