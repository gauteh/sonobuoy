/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# pragma once

# include "wirish.h"

# include "gps.h"
# include "rf.h"
# include "ads1282.h"

/* Version and settings */

/* Define to have debug messages sent to USB serial */
# define DIRECT_SERIAL 1

namespace Buoy {
  class BuoyMaster {
    public:

      GPS gps;
      ADS1282 ad;
      RF rf;


      BuoyMaster ();
      void main ();

    private:
      void setup ();
  };

  extern BuoyMaster *bu;
}


/* vim: set filetype=arduino :  */

