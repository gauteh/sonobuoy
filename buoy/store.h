/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-02-05
 *
 * SD store
 *
 */

# pragma once

# include "wirish.h"
# include "types.h"

/* SDIO, connected to SD socket.
 *
 * SDIO_D0          = 98
 * SDIO_D1          = 99
 * SDIO_D2          = 111
 * SDIO_D3          = 112
 *
 * SDIO_CK (clock)  = 113
 * SDIO_CMD         = 116
 */


namespace Buoy {
  class Store {
    public:
      RF      *rf;
      ADS1282 *ad;

      Store ();

      void setup (BuoyMaster *);
  };
}

/* vim: set filetype=arduino :  */

