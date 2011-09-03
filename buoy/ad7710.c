/* Author: Gaute Hope <eg@gaute.vetsj.com>
 *
 *
 */

# ifndef AD7710
# define AD7710

# include "buoy.h"

# define BYTES_PER_READ   3

/* PIN setup */
# define A0     46
# define nDRDY  47
# define nRFS   44
# define nTFS   45
# define SCLK   52
# define SDATA  50

void          configure ();
void          ad_setup ();
unsigned long sample ();

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

  // Notch frequency (12 bits)
  # define FREQUENCY 326


  // Build control bits
  ulong ctb = 0;

  ctb  = (ulong)(CONTROL_24BIT) << 12;
  ctb += FREQUENCY;

  Serial.print ("[AD7710] Sending configuration: ");
  Serial.println (ctb, BIN);


  for (int i = 2; i >= 0; i--) {

    byte b = ctb >> (8*i);
    shiftOut (SDATA, SCLK, MSBFIRST, b);
  }

  delay (10);
  digitalWrite (nTFS, HIGH);
  digitalWrite (A0, HIGH);
  delay (10);

  /* Waiting for calibration to finish */
  while (digitalRead(nDRDY) == HIGH) delay(10);

  pinMode(SDATA, INPUT);

  Serial.println ("[AD7710] Calibration finished, ready for normal operation.");

}

unsigned long sample ()
{
  unsigned long r = 0;

  byte b;

  digitalWrite (nRFS, LOW);
  delayMicroseconds(10);

  int i = 0;
  do {
    b = shiftIn (SDATA, SCLK, MSBFIRST);
    r += b;

    i++;

    if (i < BYTES_PER_READ)
      r <<= 8;

  } while (i < BYTES_PER_READ);

  delay (10);
  digitalWrite (nRFS, HIGH);

  return r;
}

unsigned long verbose_sample ()
{
  Serial.print ("[AD7710] Sampling, value: ");

  unsigned long r = sample ();

  Serial.print (r, BIN);
  Serial.print ("(");
  Serial.print (r, HEX);
  Serial.println (")");

  return r;
}

# endif

