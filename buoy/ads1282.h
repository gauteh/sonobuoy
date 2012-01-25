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

/* Register 1 */
/* Inputs */
# define AD_I2C_MFLAG   0b10000000

/* Inputs and outputs */
# define AD_I2C_M0      0b00000001 // GPIO0 J5.2
# define AD_I2C_M1      0b00000010 // GPIO1 J5.1
# define AD_I2C_MCLK    0b00000100 // GPIO2 J5.8

/* Outputs */
# define AD_I2C_SUPSOR  0b00001000 // GPIO3 (Power supply regulator) J5.12
# define AD_I2C_PDWN    0b00010000 // GPIO4 J5.14
# define AD_I2C_EXTCLK  0b00100000 // GPIO5 J5.19 (Controlled by jumper: On-

# define AD_I2C_SYNC    0b01000000

/* Register 2, outputs */
# define AD_I2C_PMODE   0b00000001
# define AD_I2C_RESET   0b00000010

/* Outputs configured HIGH */
# define AD_I2C_OUTPUT0 AD_I2C_SYNC
# define AD_I2C_OUTPUT1 AD_I2C_RESET

/* Control register of PCA9535RGE:
 * HIGH is input
 * LOW  is output (default)
 *
 * Programming all outputs using default (pull up) value as inputs to avoid
 * conflicting U7 output with hardwired output - meaning shortening.
 */
# define AD_I2C_CONTROL0 AD_I2C_MFLAG | AD_I2C_EXTCLK
# define AD_I2C_CONTROL1 AD_I2C_PMODE


  /* SPI */
# define AD_SPI  1
# define AD_SCLK BOARD_SPI1_SCK_PIN   // 53
# define AD_DOUT BOARD_SPI1_MISO_PIN  // 55
# define AD_DIN  BOARD_SPI1_MOSI_PIN  // 54
# define AD_SS   BOARD_SPI1_NSS_PIN   // 52

# define AD_nDRDY 40

  typedef uint32_t sample;

  class ADS1282 {
    public:
      bool disabled;
      static HardwareSPI spi;

      typedef struct _control {
      /* Control registers on EVM {{{ */
        /* Control registers of U7 / PCA9535RGE */

        /* Configured ports
         * LOW  is output
         * HIGH is input
         */
        uint8 ports0;
        uint8 ports1;

        /* Inputs */
        bool mflag; // Not reading..

        /* Outputs */
        bool sync;
        //bool pmode;
        bool reset;
        bool pdwn;

        /* Control registers of ADS1282 */
        // }}}
      } control;

      control state;

      bool batchready;
      volatile sample value;

      ADS1282 ();
      void setup ();
      void configure ();
      void reset ();
      void reset_spi ();
      void read_u7_outputs ();

      void loop ();
      void acquire ();
      static void drdy ();
      static void drdy_off ();

      void error ();

  };
}

/* vim: set filetype=arduino :  */

