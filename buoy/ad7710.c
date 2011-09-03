/* Author: Gaute Hope <eg@gaute.vetsj.com>
 *
 * Controls and interfaces the AD7710 over three-wire SPI.
 *
 */

# ifndef AD7710
# define AD7710

# include "buoy.h"

/* PIN setup */
# define A0     46
# define nDRDY  47
# define nRFS   44
# define nTFS   45
# define SCLK   52
# define SDATA  50

void          configure ();
void          ad_setup ();
ulong         sample ();

volatile ulong samples = 0;

void ad_setup ()
{
  Serial.println ("[AD7710] Setting up AD7710..");

  /* Setting up pins */
  pinMode (nDRDY, INPUT);
  pinMode (A0, OUTPUT);
  pinMode (nRFS, OUTPUT);
  pinMode (nTFS, OUTPUT);
  pinMode (SCLK, OUTPUT);
  pinMode (SDATA, INPUT);

  /* Configure AD */
  digitalWrite (A0, HIGH);
  digitalWrite (nRFS, HIGH);
  digitalWrite (nTFS, HIGH);


  /* Get control register */
  digitalWrite (A0, LOW);
  delay(1);

  Serial.print ("[AD7710] Initial control register: ");
  Serial.println (sample (), BIN);

  digitalWrite (A0, HIGH);
  delay (1);

  configure ();

  /* Get control register */
  digitalWrite (A0, BIN);
  delay(1);

  Serial.print ("[AD7710] Configured control register: ");
  Serial.println (sample (), BIN);

  digitalWrite (A0, HIGH);
  delay (1);
}

void configure ()
{
  /* Configure AD */
  pinMode (SDATA, OUTPUT);
  digitalWrite (A0, HIGH);
  digitalWrite (nRFS, HIGH);
  digitalWrite (nTFS, HIGH);

  /* Activate self-calibration */
  Serial.println ("[AD7710] Configure and activate self-calibration..");

  delay(100);
  digitalWrite (nTFS, LOW);
  digitalWrite (A0, LOW);
  delay (100);

  // Self-calibration mode:
  // Control:   0b001 000 0 0 1 0 0 0
  // Frequency: 2000 (range 19 to 2000 decimal)

  // Configuration masks (12 bits)
  # define CONTROL_SELF_CALIBRATION   0b001000000000
  # define CONTROL_24BIT              0b000000001000
  # define CONTROL_DEFAULT            0

  /* Notch frequency (12 bits)
   *
   * Range: 19 - 2000L
   * 19   gives approximately 1000 samples / 970 ms
   * 2000 gives approximately 1000 samples / 50 s
   */
  # define FREQUENCY 19L


  // Build control bits
  ulong ctb = 0;

  ctb  = (ulong) (CONTROL_SELF_CALIBRATION + CONTROL_24BIT) << 12;
  ctb += (ulong) FREQUENCY;

  Serial.print ("[AD7710] Sending configuration: ");
  Serial.println (ctb, BIN);


  for (int i = 2; i >= 0; i--) {

    byte b = ctb >> (8*i);
    shiftOut (SDATA, SCLK, MSBFIRST, b);
  }

  delay (1);
  digitalWrite (nTFS, HIGH);
  digitalWrite (A0, HIGH);
  delay (1);

  /* Waiting for calibration to finish */
  while (digitalRead(nDRDY) == HIGH) delay(1);

  pinMode(SDATA, INPUT);

  Serial.println ("[AD7710] Calibration finished, ready for normal operation.");

}

/* Get sample from AD, will return latest value even if it has been
 * read before. Use sample (true) for a blocking version.
 */
ulong sample ()
{
  ulong r = 0;

  digitalWrite (nRFS, LOW);
  delayMicroseconds(10);

  /* Shift 24 bits = 3 bytes */

  int i = 0;
  do {
    r += shiftIn (SDATA, SCLK, MSBFIRST);

    i++;

    if (i < 3)
      r <<= 8;

  } while (i < 3);

  digitalWrite (nRFS, HIGH);

  return r;
}

ulong verbose_sample ()
{
  Serial.print ("[AD7710] Sampling, value: ");

  ulong r = sample ();

  Serial.print (r, BIN);
  Serial.print ("(");
  Serial.print (r, HEX);
  Serial.println (")");

  return r;
}

ulong sample (bool blocking)
{
  if (blocking)
    while (digitalRead(nDRDY) == HIGH) delayMicroseconds(10);

  return sample ();
}

void sample_performance_test ()
{
  Serial.println ("[AD7710] Sample performance test");
  Serial.println ("[AD7710] Timing 1000 samples..");

  ulong end = 0;
  ulong start = micros ();

  for (int i = 1001; i > 0; i--) {
    while (digitalRead(nDRDY) == HIGH) ; // run in loop until data ready
    sample ();                           // sample
  }

  end = micros ();
  ulong total = (ulong)(end - start) / 1000.0L;
  
  Serial.print ("[AD7710] Duration [ms]: ");
  Serial.println (total);
}



# endif

