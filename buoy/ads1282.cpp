/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * ADS1282 driver.
 *
 */

# include "wirish.h"

# include "spi.h"
# include "Wire.h"

# include "buoy.h"
# include "ads1282.h"

using namespace std;

namespace Buoy {

  HardwareSPI ADS1282::spi (AD_SPI);


  ADS1282::ADS1282 () {
    batchready  = false;
    drdy        = false;
    value       = 0;

    state.reg1 = 0;
    state.reg2 = 0;

    state.mflag = false;
    state.sync  = false;
    state.pmode = false;
    state.reset = false;
    state.pdwn  = false;

    return;
  }

  void ADS1282::setup () {

    delay(5000);

    /* Setup AD and get ready for data */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Setting up ADS1282..");
# endif

    /* Set up I2C */
    Wire.begin (AD_SDA, AD_SCL);

    /* Set up SPI */
    pinMode (AD_nDRDY, INPUT);
    pinMode (AD_SS, OUTPUT);

    spi.begin (SPI_1_125MHZ, MSBFIRST, SPI_MODE_0);
    digitalWrite (AD_SS, LOW);

    /* Configure AD */
    configure ();
  }

  void ADS1282::reset_spi () {
    /* Hold SCLK low for 64 nDRDY cycles */
  }

  void ADS1282::loop () {
    drdy = !digitalRead (AD_nDRDY);
  }

  void ADS1282::configure () {
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Configuring ADS1282..");
# endif

    int n = 0;

    /* Configure I2C (U7) */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x06);
    Wire.send (AD_I2C_CONTROL1);
    Wire.send (AD_I2C_CONTROL2);
    n = Wire.endTransmission ();

    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }

    /* Set up outputs:
     * - Turn off SYNC
     * - Turn off PDWN
     * - Turn on  PMODE
     * - Turn off RESET
     */
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x02);
    Wire.send (AD_I2C_OUTPUT0);
    Wire.send (AD_I2C_OUTPUT1);
    n = Wire.endTransmission ();

    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }

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
      state.pmode = (r & AD_I2C_PMODE);
      state.reset = (r & AD_I2C_RESET);
    }

    n = Wire.endTransmission ();
    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }

    // Read input
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x00);
    n = Wire.endTransmission ();
    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }

    Wire.beginTransmission (AD_I2C_ADDRESS);
    n = Wire.requestFrom (AD_I2C_ADDRESS, 2);
    SerialUSB.println (n);
    while (Wire.available ()) {

      uint8 r = Wire.receive ();

      SerialUSB.print  ("[AD] [I2C] Got:");
      SerialUSB.println (r, BIN);
    }

    n = Wire.endTransmission ();
    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }

    // Read configuration
    Wire.beginTransmission (AD_I2C_ADDRESS);
    Wire.send (0x06);
    n = Wire.endTransmission ();
    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }

    Wire.beginTransmission (AD_I2C_ADDRESS);
    n = Wire.requestFrom (AD_I2C_ADDRESS, 2);
    SerialUSB.println (n);
    while (Wire.available ()) {

      uint8 r = Wire.receive ();

      SerialUSB.print  ("[AD] [I2C] Got:");
      SerialUSB.println (r, BIN);
    }

    n = Wire.endTransmission ();
    if (n == SUCCESS) SerialUSB.println ("SUCCESS");
    else {
      SerialUSB.print ("ERROR: ");
      SerialUSB.println (n);
    }
  }

  void ADS1282::read_control_register () {
    /* Not implemented */
  }

  void ADS1282::acquire () {
    /* In continuous mode: Must complete read operation before four
     *                     CLK (ADS1282) periods. */
    SerialUSB.println ("[AD] Acquiring..");


    /* Data is sent with MSB first */
    //value = spi->read ();
    //value = spi->read ();

    if (!spi_is_rx_nonempty (SPI1)) {
      SerialUSB.println ("[AD] SPI is empty.");
    } else {
      byte a = spi.read ();
      SerialUSB.print ("[AD] Got value: ");
      SerialUSB.println (a);
    }

  }
}

/* vim: set filetype=arduino :  */

