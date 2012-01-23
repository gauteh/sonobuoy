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
   * PCA9535RGE.
   */

# define AD_I2C  1
# define AD_SCL 38
# define AD_SDA 39
# define AD_I2C_ADDRESS_R 0x20 // For read operations
# define AD_I2C_ADDRESS_W 0x20 // For write operations

/* Control register of PCA953RGE:
 * HIGH is input
 * LOW  is output
 *
 * default is output.
 */

/* Inputs, register 1 */
# define AD_I2C_MFLAG 0b10000000

/* Outputs, register 1 */
# define AD_I2C_SYNC  0b01000000

/* Register 2 */
# define AD_I2C_PMODE 0b00000001
# define AD_I2C_RESET 0b00000010

# define AD_I2C_CONTROL1 AD_I2C_MFLAG
# define AD_I2C_CONTROL2 0


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

