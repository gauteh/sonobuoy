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
    run = 0;

    state.ports0 = 0;
    state.ports1 = 0;
    state.polarity0 = 0;
    state.polarity1 = 0;

    state.sync  = false;
    state.reset = false;
    state.pdwn  = false;

    for (int i = 0; i < 11; i++) reg.raw[i] = 0;

    batchready  = false;
    value       = 0;
    memset ((void*) values, 0, QUEUE_LENGTH * sizeof (uint32_t));
    memset ((void*) times, 0, QUEUE_LENGTH * sizeof (uint32_t));
    position    = 0;
    totalsamples = 0;

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

    digitalWrite (AD_SS, LOW);
    digitalWrite (AD_SCLK, LOW);
    digitalWrite (AD_DIN, LOW);

    /* Configure AD */
    configure ();

    attachInterrupt (AD_nDRDY,&(ADS1282::drdy), FALLING);

    // }}}
  }

  void ADS1282::loop () {
    /* Run as part of main loop {{{ */
    if (!disabled) {
      run++;
      //acquire_on_command ();

      /*
      SerialUSB.print ("[AD] Loop: ");
      SerialUSB.print (run);
      */


      SerialUSB.print ("[AD] Queue pos: ");
      SerialUSB.print (position);

      SerialUSB.print (", samples: ");
      SerialUSB.print (totalsamples);

      SerialUSB.print (", value: 0x");
      SerialUSB.println (value, HEX);
    } // }}}
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
     * - SUPSOR: LOW   (bipolar mode)
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

    SerialUSB.println ("[AD] Reset by command and stop read data continuous..");
    send_command (RESET);
    delay (100);

    send_command (SDATAC);
    delay (100);

    read_registers ();
    configure_registers (); // resets ADC, 63 data cycles are lost..
    delay (100);
    read_registers ();

    delay (400);

    send_command (SYNC);
    send_command (RDATAC);


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
    /* Reset SPI interface: Hold SCLK low for 64 nDRDY cycles  {{{*/
    SerialUSB.println ("[AD] [SPI] Resetting SPI..");
    digitalWrite (AD_SCLK, LOW);

    /* Make sure data is read continuously and nDRDY interrupt is detached */

    for (int i = 0; i < 64; i++) {
      while (digitalRead (AD_nDRDY)) delayMicroseconds (2);
      while (!digitalRead (AD_nDRDY)) delayMicroseconds (2);
    }

    // }}}
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
    SerialUSB.print   ("[AD] [SPI] Sending command: [");

    // String representation of command {{{
    switch (cmd) {
      case WAKEUP:
        SerialUSB.print   ("WAKEUP");
        break;
      case STANDBY:
        SerialUSB.print   ("STANDBY");
        break;
      case SYNC:
        SerialUSB.print   ("SYNC");
        break;
      case RDATAC:
        SerialUSB.print   ("RDATAC");
        break;
      case SDATAC:
        SerialUSB.print   ("SDATAC");
        break;
      case RDATA:
        SerialUSB.print   ("RDATA");
        break;
      case OFSCAL:
        SerialUSB.print   ("OFSCAL");
        break;
      case GANCAL:
        SerialUSB.print   ("GANCAL");
        break;
      case RESET:
        SerialUSB.print   ("RESET");
        break;
      case RREG:
        SerialUSB.print   ("RREG");
        break;
      case WREG:
        SerialUSB.print   ("WREG");
        break;
    }; // }}}

    SerialUSB.print   ("] 0b");
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
        SerialUSB.print   ("[AD] [SPI] Sending: 0b");
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

      /* TODO: Register values seem to arrive a bit earlier than sample values,
       * probably loosing MSB here though at the moment.. */
      //reg.raw[i] >>= 1;

      SerialUSB.print   ("[AD] Register [");
      SerialUSB.print   (i);
      SerialUSB.print   ("] 0b");
      SerialUSB.print   (reg.raw[i], BIN);
      SerialUSB.print   (" (0x");
      SerialUSB.print   (reg.raw[i], HEX);
      SerialUSB.println (")");

      switch (i)
      {
        case 0:
          reg.id = reg.raw[0];
          break;

        case 1:
          reg.sync = reg.raw[1] & (1 << 7);
          reg.mode = reg.raw[1] & (1 << 6);
          reg.datarate = (reg.raw[1] & 0b00111000) >> 3;
          reg.firphase = reg.raw[1] & (1 << 2);
          reg.filterselect = reg.raw[1] & 0b11;
          break;

        case 2:
          reg.muxselect = (reg.raw[2] & 0b01110000) >> 4;
          reg.pgachop = reg.raw[2] & 0b00001000;
          reg.pgagain = reg.raw[2] & 0b00000111;
          break;

        case 3:
          reg.hpf0 = reg.raw[3];
          break;
        case 4:
          reg.hpf1 = reg.raw[4];
          break;

        case 5:
          reg.ofc0 = reg.raw[5];
          break;
        case 6:
          reg.ofc1 = reg.raw[6];
          break;
        case 7:
          reg.ofc2 = reg.raw[7];
          break;

        case 8:
          reg.fsc0 = reg.raw[8];
          break;
        case 9:
          reg.fsc1 = reg.raw[9];
          break;
        case 0xa:
          reg.fsc2 = reg.raw[0xa];
          break;
      };

    }
    // }}}
  }

  void ADS1282::configure_registers () {
    /* Configure ADS1282 registers {{{ */
    SerialUSB.println ("[AD] Configuring registers..");

    // Config 0, changes from default:
    // - Sample rate: 250
# define AD_CONFIG0 0b01000010
    send_command (WREG, 1, 0);
    SerialUSB.print   ("[AD] [SPI] Sending: 0b");
    SerialUSB.println (AD_CONFIG0, BIN);
    shift_out (AD_CONFIG0);

    // High pass filter configuration
    // HPF[1:0] = 0x0337 => fHP = 0.5 Hz @ 250 SPS
# define AD_HPF1 0x03
# define AD_HPF0 0x32
    send_command (WREG, 3, 1);
    SerialUSB.print   ("[AD] [SPI] Sending: 0b");
    SerialUSB.println (AD_HPF0, BIN);
    shift_out (AD_HPF0);
    SerialUSB.print   ("[AD] [SPI] Sending: 0b");
    SerialUSB.println (AD_HPF1, BIN);
    shift_out (AD_HPF1);

    // }}}
  }

  // Acquire {{{
  void ADS1282::drdy () {
    /* Static wrapper function for interrupt */
    bu->ad.acquire ();
  }

  void ADS1282::acquire () {
    /* In continuous mode: Must complete read operation before four
     *                     DRDY (ADS1282) periods. */

    uint8_t v[4];
    shift_in_n (v, 4);

    /*
     * Data is formatted in twos complement.
     *
     * In sinc filter mode output is scaled by 1/2.
     */

    value  = 0;
    value |= v[0] << 24;
    value |= v[1] << 16;
    value |= v[2] << 8;
    value |= v[3];

    /* LSB of value is a redundant sign bit unless output is clipped to +/- FS
     * then it is 1 for +FS and 0 for -FS.
     */

    /* Fill batch */
    values[bu->ad.position] = value;
    times [bu->ad.position] = micros(); // Does not change value within interrupt

    position++;
    totalsamples++;

    if (position == (QUEUE_LENGTH / 2)) {
      batchready = true;
    } else if (position == QUEUE_LENGTH) {
      batchready = true;
      position = 0;
    }
  }

  void ADS1282::acquire_on_command () {
    send_command (RDATA);

    /* Wait for falling nDRDY, time out after 5 secs */
    uint32_t start = millis ();
    while (digitalRead (AD_nDRDY)) if ((millis () - start)  > 5000) return;

    // Shift bits in (should wait min 100 ns)
    acquire ();

    SerialUSB.print   ("[AD] Value: ");
    SerialUSB.println (value, HEX);
  }
  // }}}

  /* SPI clocking operations: in and out {{{ */
  uint8_t ADS1282::shift_in () {
    /* Read each bit, MSB first */
    uint8_t v = 0;
    for (int i = 7; i >= 0; i--) {
      digitalWrite (AD_SCLK, HIGH);
      digitalWrite (AD_SCLK, LOW);

      v |= (((uint8_t)digitalRead (AD_DOUT)) << (i-1));
    }

    return v;
  }

  void ADS1282::shift_in_n (uint8_t *v, int n) {
    /* Shift in n bytes to byte array v */

    /* TODO: Shifts bytes a bit too much.. seems to be correct for data values,
     * but incorrect for register values. */

    /* Read each byte */
    for (int j = 0; j < n; j++) {

      v[j] = 0;

      /* Read each bit, MSB first */
      for (int i = 0; i < 8; ++i) {
        digitalWrite (AD_SCLK, HIGH);

        v[j] |= (((uint8_t)digitalRead (AD_DOUT)) << (7 - i));
        digitalWrite (AD_SCLK, LOW);
      }

      // TODO: delay doesn't work inside interrupts; verify returned data
      if (j < n) delayMicroseconds (11); // delay, min: 24 / fclk
    }
  }

  void ADS1282::shift_out (uint8_t v, bool delay) {
    /* Write each bit, MSB first */
    for (int i = 7; i >= 0; i--) {
      digitalWrite (AD_DIN, !!(v & (1 << i)));
      digitalWrite (AD_SCLK, HIGH);
      digitalWrite (AD_SCLK, LOW);
    }

    digitalWrite (AD_DIN, LOW);
    if (delay) delayMicroseconds (11); // delay, min: 24 / fclk, required if
                                       // reading or sending more commands.
  }

  // }}}

  void ADS1282::error () {
    /* Some error on the ADS1282 - disable {{{ */
# if DIRECT_SERIAL
    SerialUSB.println ("[AD] Error. Disabling.");
# endif

    disabled = true;
    detachInterrupt (AD_nDRDY);
    // }}}
  }
}

/* vim: set filetype=arduino :  */

