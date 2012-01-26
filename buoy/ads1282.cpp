/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# include <string.h>
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
    run = 0;

    state.ports0 = 0;
    state.ports1 = 0;
    state.polarity0 = 0;
    state.polarity1 = 0;

    state.sync  = false;
    state.reset = false;
    state.pdwn  = false;

    for (int i = 0; i < 11; i++) reg.raw[i] = 0;

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
    pinMode (AD_DOUT, INPUT_PULLDOWN);
    pinMode (AD_nDRDY, INPUT_PULLDOWN);
    pinMode (AD_SS, OUTPUT);

    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
    attachInterrupt (AD_nDRDY, (&ADS1282::drdy), CHANGE);

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
      run++;

      /*
      SerialUSB.print ("[AD] Run: ");
      SerialUSB.println (run);
      */

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

    /* Set up outputs: (defined in header file)
     * - SYNC:   HIGH  (active low)
     * - PDWN:   HIGH  (active low)
     * - RESET:  HIGH  (active low)
     * - M0:     LOW
     * - M1:     HIGH
     * - MCLK:   LOW
     * - SUPSOR: LOW
     * - EXTCLK: HIGH  (also hardwired high: _must_ be set HIGH)
     *
     * All other U7 outputs are meanwhile configured as inputs:
     * - PMODE (not available on ADS1282)
     * - MFLAG
     */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_pca9535 (OUTPUT0);
    delay (100); // Allow EVM and AD to power up..

    reset ();
    delay (100);

    SerialUSB.println ("[AD] Send: RESET..");
    send_command (RESET);
    delay (100);

    SerialUSB.println ("[AD] Send: SDATAC..");
    send_command (SDATAC);
    delay (100);

    read_registers ();

    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
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
    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
    delay (1000);
    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));

    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_pca9535 (OUTPUT0);
    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
    delay (100);


    SerialUSB.println ("[AD] Reset done.");
    // }}}
  }

  void ADS1282::send_command (COMMAND cmd, uint8_t start, uint8_t n) {
    /* Send SPI command to ADS1282 {{{ */
    SerialUSB.print   ("[AD] Sending command: 0b");
    SerialUSB.println ((uint8_t) (cmd + start), BIN);
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
        shift_out ((uint8_t) cmd);
        break;

      case RREG:
      case WREG:
        SerialUSB.print   ("[AD] Sending command: 0b");
        SerialUSB.println ((uint8_t) (n), BIN);
        shift_out ((uint8_t) (cmd + start));
        shift_out (n);
        break;
    };
    // }}}
  }

  void ADS1282::read_registers () {
    /* Read registers of ADS1282, SDATAC must allready have been issued {{{ */
    SerialUSB.println ("[AD] Reading registers..");
    send_command (RREG, 0, 10);

    shift_in_n (reg.raw, 11);

    for (int i = 0; i < 11; i++) {
      SerialUSB.print   ("[AD] Register [");
      SerialUSB.print   (i);
      SerialUSB.print   ("] 0b");
      SerialUSB.print   (reg.raw[i], BIN);
      SerialUSB.print   (" (0x");
      SerialUSB.print   (reg.raw[i], HEX);
      SerialUSB.println (")");
    }
    // }}}
  }

  void ADS1282::drdy () {
    digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));
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

    shift_in_n ((uint8_t*) &value, 4);

    SerialUSB.println (value, HEX);

    // }}}
  }

  /* SPI clocking operations: in and out {{{ */ 
  uint8_t ADS1282::shift_in () {
    digitalWrite (AD_DIN, LOW);
    digitalWrite (AD_SCLK, LOW);

    /* Read each bit, MSB first */
    uint8_t v = 0;
    for (int i = 7; i >= 0; i--) {
      digitalWrite (AD_SCLK, HIGH);
      digitalWrite (AD_SCLK, LOW);

      v |= digitalRead (AD_DOUT) << i;
    }

    return v;
  }

  void ADS1282::shift_in_n (uint8_t *v, int n) {
    /* Shift in n bytes to byte array v */
    digitalWrite (AD_DIN, LOW);
    digitalWrite (AD_SCLK, LOW);

    /* Read each byte */
    for (int j = 0; j < n; j++) {

      v[j] = 0;

      /* Read each bit, MSB first */
      for (int i = 7; i >= 0; i--) {
        digitalWrite (AD_SCLK, HIGH);
        digitalWrite (AD_SCLK, LOW);

        v[j] |= (((uint8_t)digitalRead (AD_DOUT)) << (i-1));
      }

      delayMicroseconds (11); // delay, min: 24 / fclk
    }
  }

  void ADS1282::shift_out (uint8_t v) {
    digitalWrite (AD_DIN, LOW);
    digitalWrite (AD_SCLK, LOW); // hopefully already there..

    /* Write each bit, MSB first */
    for (int i = 7; i >= 0; i--) {
      digitalWrite (AD_DIN, !!(v & (1 << i)));
      digitalWrite (AD_SCLK, HIGH);
      digitalWrite (AD_SCLK, LOW);
    }

    delayMicroseconds (11); // delay, min: 24 / fclk
  }

  // }}}

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

