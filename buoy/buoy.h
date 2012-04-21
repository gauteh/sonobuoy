/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# pragma once

/* Version and settings */
# define VERSION GIT_DESC

/* ID for this Buoy */
# define BUOY_ID    1
# define BUOY_NAME  "One"

# ifndef ONLY_SPEC

# include <stdint.h>

/* Disable any ASSERT() and serial line debugger (saves some program size)
 *
 * Must be defined before any wirish.h or libmaple.h includes.
 *
 */
//# define DEBUG_LEVEL DEBUG_NONE

# include "wirish.h"

# include "types.h"

/* Print debug messages to USB serial */
# define DIRECT_SERIAL 0

/* Macros for stringifying defines */
# define STRINGIFY_I(s) #s
# define STRINGIFY(s)   STRINGIFY_I(s)

namespace Buoy {
  class BuoyMaster {
    public:
      GPS       *gps;
      ADS1282   *ad;
      RF        *rf;
      Store     *store;

      BuoyMaster ();
      void main ();
      void send_greeting ();

    private:
      void setup ();
  };

  int itoa (uint32_t, uint8_t, char *);

  /* Globally available instance of BuoyMaster */
  extern BuoyMaster *bu;
}

# endif

/* vim: set filetype=arduino :  */

