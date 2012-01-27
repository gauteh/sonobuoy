/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# pragma once

# include <stdint.h>
# include <string>

using namespace std;

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
//# define AD_I2C_PMODE   0b00000001 // Only available on ADS1281
# define AD_I2C_RESET   0b00000010

/* Outputs configured HIGH */
# define AD_I2C_OUTPUT0 AD_I2C_SYNC | AD_I2C_PDWN | AD_I2C_M1 | AD_I2C_EXTCLK
# define AD_I2C_OUTPUT1 AD_I2C_RESET

/* Control register of PCA9535RGE:
 * HIGH is input
 * LOW  is output (default)
 *
 * Programming all outputs using default (pull up) value as inputs to avoid
 * conflicting U7 output with hardwired output - meaning shortening.
 */
# define AD_I2C_CONTROL0 AD_I2C_MFLAG
# define AD_I2C_CONTROL1  0
# define AD_I2C_POLARITY0 0
# define AD_I2C_POLARITY1 0


  /* SPI */
# define AD_SPI   1
# define AD_SCLK 53
# define AD_DOUT 55
# define AD_DIN  54
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

        uint8 polarity0;
        uint8 polarity1;

        /* Outputs */
        bool sync;
        //bool pmode;
        bool reset;
        bool pdwn;

        /* Control registers of ADS1282 */
        // }}}
      } control;
      control state;

      typedef struct _registers {
        /* Registers of ADS1282 {{{ */
        uint8_t raw[11];

        // ID (address: 0x00)
        uint8_t id;

        // Config 0 (address: 0x01)
        bool    sync;     // pulse (default) | continuous
        bool    mode;     // low power | high resolution (default)
        uint8_t datarate; // 0 - 5: - 250
                          //        - 500
                          //        - 1000 (default)
                          //        - 2000
                          //        - 4000
        bool    firphase; // linear phase (default) | minimum phase
        uint8_t filterselect; // 0 - 3: - On-chip filter bypassed
                              //        - Sinc filter block only
                              //        - Sinc + LPF filter blocks (default)
                              //        - Sinc + LPF + HPF filter blocks


        // Config 1 (address: 0x02)
        // First bit reserved (always write 0)
        uint8_t muxselect; // 0 - 4: - AINP1 and AINN1 (default)
                           //        - AINP2 and AINN2
                           //        - Internal short via 400 Ohm
                           //        - AINP1 and AINN1 conn to AINP2 and AINN2
                           //        - External short to AINN2
        bool    pgachop;   // disabled | enabled (default)
        uint8_t pgagain;   // 0 - 6: - G = 1 (default)
                           //        - G = 2
                           //        - G = 4
                           //        - G = 8
                           //        - G = 16
                           //        - G = 32
                           //        - G = 64

        // High pass filter (address: 0x03 -0x04)
        // Two bytes set the corner frequency of the High Pass filter
        uint8_t hpf0; // low byte,  default 0x32
        uint8_t hpf1; // high byte, default 0x03

        // Offset calibration (address: 0x05 - 0x07)
        uint8_t ofc0; // low byte,  default 0x00
        uint8_t ofc1; // mid byte,  default 0x00
        uint8_t ofc2; // high byte, default 0x00

        // Full scale calibration (address: 0x08 - 0x0a)
        uint8_t fsc0; // low byte,  default 0x00
        uint8_t fsc1; // mid byte,  default 0x00
        uint8_t fsc2; // high byte, default 0x00

        // }}}
      } registers;

      registers reg;

      typedef enum _pca9535register {
        /* Register id, corresponds to register id on device {{{ */
        INPUT0 = 0,
        INPUT1,
        OUTPUT0,
        OUTPUT1,
        POLARITY0,
        POLARITY1,
        CONTROL0,
        CONTROL1
        // }}}
      } PCA9535REGISTER;

      typedef enum _command {
      /* SPI commands for ADS1282 {{{ */
        WAKEUP    = 0x00,
        STANDBY   = 0x02,
        SYNC      = 0x04,
        RESET     = 0x06,
        RDATAC    = 0x10, // Read data continuous
        SDATAC    = 0x11, // Stop read data continuous
        RDATA     = 0x12, // Read data on command (in SDATAC mode)

        // Read and write registers
        RREG      = 0x20, // + starting address, and second byte with number
                          // of register to be read -1.
        WREG      = 0x40, // ^^

        OFSCAL    = 0x60, // Offset calibration
        GANCAL    = 0x61  // Gain calibration
        // }}}
      } COMMAND;

      bool batchready;
      volatile sample value;

      int run;

      ADS1282 ();
      void setup ();
      void configure ();
      void reset ();
      void reset_spi ();
      void read_pca9535 (PCA9535REGISTER);

      void send_command (COMMAND cmd, uint8_t start = 0, uint8_t n = 0);
      void read_registers ();
      void configure_registers ();

      void loop ();
      void acquire ();
      void acquire_on_command ();
      static void drdy ();
      static void drdy_off ();

      void    shift_out (uint8_t v);
      uint8_t shift_in  ();
      void    shift_in_n (uint8_t *, int);

      void error ();

  };
}

/* vim: set filetype=arduino :  */

