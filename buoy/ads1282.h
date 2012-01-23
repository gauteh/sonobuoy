/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# pragma once

# include <stdint.h>

namespace Buoy {

  /* I2C bus
   *
   * The I2C control unit (U7 on ADS1282-EVM schematic) appears to be an
   * PCA9535RGE, address for read and write is obtained from PCA9536
   * datasheet.
   */

# define AD_I2C  1
# define AD_SCL 38
# define AD_SDA 39
# define AD_I2C_ADDRESS_R 0b10000011 // For read operations
# define AD_I2C_ADDRESS_W 0b10000010 // For write operations

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
      static HardwareSPI spi;

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

