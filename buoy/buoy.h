/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# pragma once

/* Version and settings */
# define VERSION "0.6-dev"

# ifndef ONLY_SPEC

# include "wirish.h"
# include "types.h"

/* Define to have debug messages sent to USB serial */
# define DIRECT_SERIAL 1

namespace Buoy {
  class BuoyMaster {
    public:
      GPS       *gps;
      ADS1282   *ad;
      RF        *rf;
      Store     *store;

      BuoyMaster ();
      void main ();

    private:
      void setup ();
  };

  /* Globally available instance of BuoyMaster */
  extern BuoyMaster *bu;
}

# endif

/* vim: set filetype=arduino :  */

