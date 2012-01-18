/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# pragma once

# include <stdint.h>

namespace Buoy {

  /* I2C bus */
# define AD_I2C  1
# define AD_SCL 38
# define AD_SDA 39

  /* SPI */
# define AD_SPI  1
# define AD_SCLK BOARD_SPI1_SCK_PIN   // 53
# define AD_MISO BOARD_SPI1_MISO_PIN  // 55
# define AD_MOSI BOARD_SPI1_MOSI_PIN  // 54
# define AD_SS   BOARD_SPI1_NSS_PIN   // 52

# define AD_nDRDY 40

  typedef uint32_t sample;

  class ADS1282 {
    public:
      /*HardwareSPI *spi;*/

      bool batchready;
      bool drdy;
      sample value;

      ADS1282 ();
      void setup ();
      void configure ();
      void reset_spi ();
      void read_control_register ();

      void loop ();
      void acquire ();

  };

}

/* vim: set filetype=arduino :  */

