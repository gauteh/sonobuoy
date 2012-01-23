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
# define AD_I2C_ADDRESS 0x20

/* Inputs, register 1 */
# define AD_I2C_MFLAG 0b10000000

/* Outputs, register 1 */
# define AD_I2C_M0      0b00000001 // GPIO0 J5.2
# define AD_I2C_M1      0b00000010 // GPIO1 J5.1
# define AD_I2C_MCLK    0b00000100 // GPIO2 J5.8
# define AD_I2C_SUPSOR  0b00001000 // GPIO3 (Power supply regulator) J5.12
# define AD_I2C_PDWN    0b00010000 // GPIO4 J5.14
# define AD_I2C_EXTCLK  0b00100000 // GPIO5 J5.19

# define AD_I2C_SYNC    0b01000000

/* Register 2 */
# define AD_I2C_PMODE   0b00000001
# define AD_I2C_RESET   0b00000010

/* Control register of PCA9535RGE:
 * HIGH is input
 * LOW  is output
 *
 * default is output.
 */
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

      /* Control registers on EVM */
      typedef struct _control {
        /* Control registers of U7 / PCA9535RGE */

        /* Configured ports
         * LOW  is output
         * HIGH is input
         */
        uint8 reg1;
        uint8 reg2;

        /* Inputs */
        bool mflag;

        /* Outputs */
        bool sync;
        bool pmode;
        bool reset;
        bool pdwn; // Also _hardwired_ to power on.


        /* Control registers of ADS1282 */
        // TODO
      } control;

      control state;

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

