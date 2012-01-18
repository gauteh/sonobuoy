/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# pragma once

# include <stdint.h>

namespace Buoy {

  class ADS1282 {

    typedef uint32_t sample;

    public:
      bool batchready;

      sample lastvalue;

      ADS1282 ();
      void setup ();
      void configure ();
      void read_control_register ();

      void acquire ();

  };

}

/* vim: set filetype=arduino :  */

