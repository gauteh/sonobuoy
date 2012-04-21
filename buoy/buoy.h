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
# define DIRECT_SERIAL 1
# define DEBUG_SD      0 // enable debug error messages on SdFat

# if ! (defined (BOARD_olimex_stm32_h103) || defined (BOARD_maple_native))
  # error ("BOARD must either be: olimex_stm32_h103 or maple_native")
# endif


/* Macros for stringifying defines */
# define STRINGIFY_I(s) #s
# define STRINGIFY(s)   STRINGIFY_I(s)

namespace Buoy {
  class BuoyMaster {
    public:
      /*GPS       *gps;*/
      ADS1282   *ad;
      /*RF        *rf;*/
      /*Store     *store;*/

      BuoyMaster ();
      void main ();

    private:
      void setup ();
  };

  int itoa (uint32_t, uint8_t, char *);

  /* Globally available instance of BuoyMaster */
  extern BuoyMaster *bu;
}

# endif

/* vim: set filetype=arduino :  */

