/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# pragma once

namespace Buoy {

  class BuoyMaster {
    public:
      BuoyMaster ();
      void main ();

    private:
      void setup ();
      void loop ();
  };
}

/* vim: set filetype=arduino :  */

