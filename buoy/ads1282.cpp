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


    return;
  }

  //HardwareSPI spi (1);

  void ADS1282::setup () {
    /* Setup AD and get ready for data */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Setting up ADS1282..");
# endif

    /* Set up I2C */
    Wire.begin (AD_SDA, AD_SCL);

    /* Set up SPI */
    pinMode (AD_nDRDY, INPUT);
    pinMode (AD_SS, OUTPUT);

    ADS1282::spi.begin (SPI_1_125MHZ, MSBFIRST, SPI_MODE_0);
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
    /* Not implemented */
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
      byte a = ADS1282::spi.read ();
      SerialUSB.print ("[AD] Got value: ");
      SerialUSB.println (a);
    }

  }
}

/* vim: set filetype=arduino :  */

