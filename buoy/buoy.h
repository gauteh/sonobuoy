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

/* ONLY_SPEC may be defined when including any of the headers to only include
 * any definitions and constants without code or code dependant parts. */

# ifndef ONLY_SPEC

/* Disable any ASSERT() and serial line debugger (decreases program size
 * slightly).
 *
 * Must be defined before any wirish.h or libmaple.h includes. */

//# define DEBUG_LEVEL DEBUG_NONE

# include <stdint.h>
# include "wirish.h"

# include "types.h"

/* Print debug messages to USB serial */
# define DEBUG_VERB    0
# define DEBUG_INFO    0
# define DEBUG_WARN    1
# define DEBUG_SD      0 // enable debug error messages on SdFat

# define WATCHDOG_RELOAD 120 * (40000 / 256) // ca 2 minutes
# define WATCHDOG_RELOAD_INIT (WATCHDOG_RELOAD * 2)

// Enable sections in dependant classes
# define HASRF  1
# define HASGPS 1

/* Define board, 0 = maple_native, 1 = olimexino/maple (should be
 * compatible with BOARD in Makefile) */
# define BBOARD 1

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

      void main ();

      static bool hasusb ();

    private:
      void setup ();
  };

  int itoa (uint32_t, uint8_t, char *);
  byte gen_checksum (const char *);
  bool test_checksum (const char *);
  //void append_checksum (char *);

  /* Globally available instance of BuoyMaster (i.e. for statical interrupt
   * handlers). */
  extern BuoyMaster *bu;
}

# endif

/* vim: set filetype=arduino :  */

