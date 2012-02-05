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
 *
 * SPI mode, pins:
 * CS     = CD (card detect) / SDIO_D3 (Data 3)
 * DI     = SDIO_CMD
 * SCLK   = SDIO_CK
 * DO     = SDIO_D0 (Data 0)
 */
# define SD_CS    SDIO_D3
# define SD_MOSI  SDIO_CMD
# define SD_MISO  SDIO_D0
# define SD_SCLK  SDIO_CK

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

