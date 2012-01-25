/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# include "wirish.h"

# include "Wire.h"

# include "buoy.h"
# include "ads1282.h"

using namespace std;

namespace Buoy {
  ADS1282::ADS1282 () {
    // Init class {{{
    disabled    = false;
    batchready  = false;
    value       = 0;

    state.ports0 = 0;
    state.ports1 = 0;
    state.polarity0 = 0;
    state.polarity1 = 0;

    state.sync  = false;
    state.reset = false;
    state.pdwn  = false;

    return;
    // }}}
  }

  void ADS1282::setup () {
    // Set up interface and ADS1282 {{{

    /* Setup AD and get ready for data */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Setting up ADS1282..");
# endif

    /* Set up I2C */
    Wire.begin (AD_SDA, AD_SCL);

    /* Set up SPI */
    pinMode (AD_SCLK, OUTPUT);
    pinMode (AD_DIN, OUTPUT);
    pinMode (AD_DOUT, INPUT);
    pinMode (AD_nDRDY, INPUT);
    pinMode (AD_SS, OUTPUT);

    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
    //attachInterrupt (AD_nDRDY, (&ADS1282::drdy), FALLING);

    //spi.begin (SPI_1_125MHZ, MSBFIRST, SPI_MODE_0);
    digitalWrite (AD_SS, LOW);
    digitalWrite (AD_SCLK, LOW);
    digitalWrite (AD_DIN, LOW);

    /* Configure AD */
    configure ();

    // }}}
  }

  void ADS1282::loop () {
    if (!disabled) {
      /*
      SerialUSB.print ("[AD] Current value: ");
      SerialUSB.println (value, HEX);
      */
      digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
    }
  }

  void ADS1282::configure () {
    // Configure {{{
    SerialUSB.println ("[AD] Configuring ADS1282..");

    int n = 0;

    /* Configure I2C (U7) */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x06);
    Wire.send (AD_I2C_CONTROL0);
    Wire.send (AD_I2C_CONTROL1);
    n = Wire.endTransmission ();

    if (n != SUCCESS) { error (); return; }

    // Read configuration
    read_pca9535 (CONTROL0);
    read_pca9535 (POLARITY0);

    /* Set up outputs:
     * - Turn off SYNC
     * - Turn off PDWN
     * - Turn off RESET
     * - Turn off M0
     * - Turn off M1
     * - Turn off MCLK
     * - Turn off SUPSOR
     *
     * All other U7 outputs are meanwhile configured as inputs:
     * - PMODE
     * - MFLAG
     * - EXTCLK
     */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_pca9535 (OUTPUT0);

    // Let device settle, then reset
    delay (100);
    reset ();

    SerialUSB.println ("[AD] Configuration done.");
    // }}}
  }

  void ADS1282::read_pca9535 (PCA9535REGISTER reg) {
    /* Read registers of PCA9535RGE {{{
     *
     * Select register, if first register: reads both, if second: only last.
     */
    int n = 0;
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (reg);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    // Read outputs
    Wire.beginTransmission (AD_I2C_ADDRESS);
    n = Wire.requestFrom (AD_I2C_ADDRESS, 2);
    if (n == 2) {
      uint8 r = 0;
      switch (reg)
      {
        case OUTPUT0:
          /* Register 1 */
          r = Wire.receive ();
          state.sync = (r & AD_I2C_SYNC);
          state.pdwn = (r & AD_I2C_PDWN);
        case OUTPUT1:
          /* Register 2 */
          r = Wire.receive ();
          //state.pmode = (r & AD_I2C_PMODE);
          state.reset = (r & AD_I2C_RESET);
          SerialUSB.print   ("[AD] Sync: ");
          SerialUSB.print   ((state.sync ? "True " : "False"));
          SerialUSB.print   (", Reset: ");
          SerialUSB.print   ((state.reset ? "True " : "False"));
          SerialUSB.print   (", Power down: ");
          SerialUSB.println ((state.pdwn ? "True " : "False"));
          break;

        case POLARITY0:
          state.polarity0 = Wire.receive ();
        case POLARITY1:
          state.polarity1 = Wire.receive ();
          SerialUSB.print   ("[AD] PCA9535 polarity: (0)[0b");
          SerialUSB.print   (state.polarity0, BIN);
          SerialUSB.print   ("] (1)[0b");
          SerialUSB.print   (state.polarity1, BIN);
          SerialUSB.println ("]");
          break;

        case CONTROL0:
          state.ports0 = Wire.receive ();
        case CONTROL1:
          state.ports1 = Wire.receive ();
          SerialUSB.print   ("[AD] PCA9535 control:  (0)[0b");
          SerialUSB.print   (state.ports0, BIN);
          SerialUSB.print   ("] (1)[0b");
          SerialUSB.print   (state.ports1, BIN);
          SerialUSB.println ("]");
          break;

        /* Skipping inputs and polarity inverts.. */
        default:
          break;
      }
    }

    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }
    // }}}
  }

  void ADS1282::reset_spi () {
    /* Reset SPI interface: Hold SCLK low for 64 nDRDY cycles */
    digitalWrite (AD_SCLK, LOW);
    delay (1000); // TODO: Long enough.. ?
  }

  void ADS1282::reset () {
    // Reset ADS1282 over I2C / U7 {{{
    SerialUSB.println ("[AD] Resetting..");

    digitalWrite (AD_SCLK, LOW); // Make sure SPI interface is reset

    /* Sequence:
     *
     * - Set nRESET low
     * - delay min 2/fclk (currently 100 ms)
     * - Set nRESET high
     *
     */

    int n = 0;

    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1 & !AD_I2C_RESET);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_pca9535 (OUTPUT0);
    delay (100);

    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_pca9535 (OUTPUT0);
    delay (100);

    SerialUSB.println ("[AD] Sending reset command..");
    send_command (RESET);
    delay (1000);

    SerialUSB.println ("[AD] Reading registers..");
    send_command (RREG, 0, 0xA);
    for (int i = 0; i < 0xA; i++) {
      delayMicroseconds (10);
      uint8_t r = shift_in ();
      SerialUSB.print  ("[AD] Register: 0b");
      SerialUSB.println (r, BIN);
    }

    SerialUSB.println ("[AD] Reset done.");
    // }}}
  }

  void ADS1282::send_command (COMMAND cmd, uint8_t start, uint8_t n) {
    /* Send SPI command to ADS1282 {{{ */
    switch (cmd) {
      case WAKEUP:
      case STANDBY:
      case SYNC:
      case RDATAC:
      case SDATAC:
      case RDATA:
      case OFSCAL:
      case GANCAL:
      case RESET:
        shift_out (cmd);
        delayMicroseconds (10);
        break;

      case RREG:
      case WREG:
        shift_out (cmd + start);
        delayMicroseconds (10);
        shift_out (n);
        delayMicroseconds (10);
        break;
    };
    // }}}
  }

  void ADS1282::drdy () {
    SerialUSB.println ("[AD] DRDY");
    digitalWrite (BOARD_LED_PIN, HIGH);
    //bu->ad.acquire ();
  }

  void ADS1282::drdy_off () {
    SerialUSB.println ("[AD] DRDY OFF");
    digitalWrite (BOARD_LED_PIN, LOW);
  }

  void ADS1282::acquire () {
    // Acquire {{{
    /* In continuous mode: Must complete read operation before four
     *                     DRDY (ADS1282) periods. */
    SerialUSB.print ("[AD] Acquiring..: ");


    /* Data is sent with MSB first */
    //value = spi->read ();
    //value = spi->read ();

    uint32_t v = 0;
    v += shift_in () << 24;
    v += shift_in () << 16;
    v += shift_in () <<  8;
    v += shift_in ();

    value = v;

    SerialUSB.println (value, HEX);

    /*
    if (!spi_is_rx_nonempty (SPI1)) {
      SerialUSB.println ("[AD] SPI is empty.");
    } else {
      byte a = spi.read ();
      SerialUSB.print ("[AD] Got value: ");
      SerialUSB.println (a);
    }
    */
    // }}}
  }

  uint8_t ADS1282::shift_in () {
    digitalWrite (AD_DIN, LOW);
    digitalWrite (AD_SCLK, LOW);

    /* Read each bit, MSB first */
    uint8_t v = 0;
    for (int i = 7; i >= 0; i--) {
      digitalWrite (AD_SCLK, HIGH);
      delayMicroseconds (1);
      digitalWrite (AD_SCLK, LOW);
      delayMicroseconds (1);

      v |= digitalRead (AD_DOUT) << i;
      delayMicroseconds (1);
    }

    return v;
  }

  void ADS1282::shift_out (uint8_t v) {
    digitalWrite (AD_DIN, LOW);
    digitalWrite (AD_SCLK, LOW); // hopefully already there..

    /* Write each bit, MSB first */
    for (int i = 7; i >= 0; i--) {
      digitalWrite (AD_SCLK, HIGH);
      delayMicroseconds (1);
      digitalWrite (AD_DIN, (v >> i) & 1);
      delayMicroseconds (1);
      digitalWrite (AD_SCLK, LOW);
      delayMicroseconds (1);
    }
  }

  void ADS1282::error () {
    /* Some error on the ADS1282 - disable */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Error. Disabling.");
# endif

    disabled = true;
    detachInterrupt (AD_nDRDY);
  }
}

/* vim: set filetype=arduino :  */

