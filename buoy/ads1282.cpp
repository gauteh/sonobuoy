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

    state.mflag = false;
    state.sync  = false;
    state.reset = false;
    state.pdwn  = false;

    return;
    // }}}
  }

  void ADS1282::setup () {
    // Set up interface and ADS1282 {{{
    delay(1000);

    /* Setup AD and get ready for data */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Setting up ADS1282..");
# endif

    attachInterrupt (AD_nDRDY, (&ADS1282::drdy), FALLING);
    attachInterrupt (AD_nDRDY, (&ADS1282::drdy_off), RISING);
    //digitalWrite (BOARD_LED_PIN, !digitalRead (AD_nDRDY));

    /* Set up I2C */
    Wire.begin (AD_SDA, AD_SCL);

    /* Set up SPI */
    pinMode (AD_SCLK, OUTPUT);
    pinMode (AD_DIN, OUTPUT);
    pinMode (AD_DOUT, INPUT);
    pinMode (AD_nDRDY, INPUT);
    pinMode (AD_SS, OUTPUT);

    //spi.begin (SPI_1_125MHZ, MSBFIRST, SPI_MODE_0);
    digitalWrite (AD_SS, LOW);
    digitalWrite (AD_SCLK, LOW);
    digitalWrite (AD_DIN, LOW);

    /* Configure AD */
    configure ();
    digitalWrite (AD_SCLK, HIGH);

    reset ();
    delay (500);
    // }}}
  }

  void ADS1282::loop () {
    if (!disabled) {
      SerialUSB.print ("[AD] Current value: ");
      SerialUSB.println (value, HEX);
    }
  }

  void ADS1282::configure () {
    // Configure {{{
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Configuring ADS1282..");
# endif

    int n = 0;

    /* Configure I2C (U7) */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x06);
    Wire.send (AD_I2C_CONTROL0);
    Wire.send (AD_I2C_CONTROL1);
    n = Wire.endTransmission ();

    if (n != SUCCESS) { error (); return; }

    // Read configuration {{{
    Wire.beginTransmission (AD_I2C_ADDRESS);
    n = Wire.requestFrom (AD_I2C_ADDRESS, 2);
    if (n == 2) {
      state.ports0 = Wire.receive ();
      state.ports1 = Wire.receive ();
    } // }}}

    /* Set up outputs:
     * - Turn off SYNC
     * - Turn off PDWN
     * - Turn off RESET
     *
     * All other U7 outputs are meanwhile configured as inputs.
     */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_u7_outputs ();
    // }}}
  }

  void ADS1282::read_u7_outputs () {
    /* Read outputs of PCA9535RGE {{{ */
    // Select outputs
    int n = 0;
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    // Read outputs
    Wire.beginTransmission (AD_I2C_ADDRESS);
    n = Wire.requestFrom (AD_I2C_ADDRESS, 2);
    if (n == 2) {
      /* Register 1 */
      uint8 r = Wire.receive ();
      state.sync = (r & AD_I2C_SYNC);
      state.pdwn = (r & AD_I2C_PDWN);

      /* Register 2 */
      r = Wire.receive ();
      //state.pmode = (r & AD_I2C_PMODE);
      state.reset = (r & AD_I2C_RESET);
    }

    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }
    SerialUSB.print   ("[AD] Sync: ");
    SerialUSB.print   ((state.sync ? "True" : "False"));
    SerialUSB.print   (", Reset: ");
    SerialUSB.print   ((state.reset ? "True" : "False"));
    SerialUSB.print   (", Power down: ");
    SerialUSB.println ((state.pdwn ? "True" : "False"));
    // }}}
  }

  void ADS1282::reset_spi () {
    // Reset SPI interface

    /* Hold SCLK low for 64 nDRDY cycles */
  }

  void ADS1282::reset () {
    // Reset ADS1282 over I2C / U7 {{{
    SerialUSB.println ("[AD] Resetting..");

    int n = 0;

    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1 & !AD_I2C_RESET);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_u7_outputs ();
    delay (100);

    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();
    if (n != SUCCESS) { error (); return; }

    read_u7_outputs ();
    delay (100);
    // }}}
  }

  void ADS1282::drdy () {
    SerialUSB.println ("[AD] DRDY");
    digitalWrite (BOARD_LED_PIN, HIGH);
    bu->ad.acquire ();
  }

  void ADS1282::drdy_off () {
    SerialUSB.println ("[AD] DRDY OFF");
    digitalWrite (BOARD_LED_PIN, LOW);
  }

  void ADS1282::acquire () {
    // Acquire {{{
    /* In continuous mode: Must complete read operation before four
     *                     DRDY (ADS1282) periods. */
    SerialUSB.println ("[AD] Acquiring..");


    /* Data is sent with MSB first */
    //value = spi->read ();
    //value = spi->read ();

    digitalWrite (AD_DIN, LOW);
    digitalWrite (AD_SCLK, LOW);
    uint32_t v = 0;
    v += shiftIn (AD_DOUT, AD_SCLK, MSBFIRST) << 24;
    v += shiftIn (AD_DOUT, AD_SCLK, MSBFIRST) << 16;
    v += shiftIn (AD_DOUT, AD_SCLK, MSBFIRST) << 8;
    v += shiftIn (AD_DOUT, AD_SCLK, MSBFIRST);

    value = v;
    digitalWrite (AD_SCLK, HIGH);

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

