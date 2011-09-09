/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Interface to the AD7710 over three-wire SPI.
 *
 * Sampling when monitoring for DRDY interrupts:
 *
 *   When monitoring nDRDY for interrupts none of the blocking sample
 *   functions will work since the value will be retrieved immediately
 *   as DRDY goes LOW and returns HIGH again before the blocking sample
 *   polls DRDY again. That means a call to those will _block forever_.
 *
 */

# ifndef AD7710
# define AD7710

# include "buoy.h"
# include "ad7710.h"

/* PIN setup */
# define A0     46
# define nDRDY  2
# define nDRDY_INTERRUPT 0
# define nRFS   44
# define nTFS   45
# define SCLK   52
# define SDATA  50

/* Private functions */
void ad_drdy ();

volatile  ulong   ad_samples = 0;
volatile  ulong   ad_value   = 0; /* Last sampled value */

ulong     ad_start = 0; // Starting time for sample

volatile  ulong   ad_queue[AD_QUEUE_LENGTH];
volatile  ulong   ad_time[AD_QUEUE_LENGTH];
volatile  int     ad_qposition;

void ad_setup ()
{
  for (int i = 0; i < AD_QUEUE_LENGTH; i++)
    ad_queue[i] = 0;

  ad_qposition = 0;

  /* Setting up pins */
  pinMode (nDRDY, INPUT);
  pinMode (A0, OUTPUT);
  pinMode (nRFS, OUTPUT);
  pinMode (nTFS, OUTPUT);
  pinMode (SCLK, OUTPUT);
  pinMode (SDATA, INPUT);


  ad_configure ();

  ad_start = millis ();

  /* Configure interrupt */
  attachInterrupt (nDRDY_INTERRUPT, ad_drdy, LOW);
}

/* Will be run on nDRDY LOW */
void ad_drdy ()
{
  ad_value = ad_sample ();
  ad_samples++;

  ad_queue[ad_qposition] = ad_value;
  ad_time[ad_qposition]  = micros ();
  ad_qposition++;

  if (ad_qposition >= AD_QUEUE_LENGTH) ad_qposition = 0;
}

ulong ad_read_control_register ()
{
  /* Note:  If output register is ready, it seems like setting A0 LOW
   *        has no effect on changing output to control register
   *        before one READ cycle has been performed and output register
   *        has been purged.
   *
   *        TODO: A single pre-sample / or double sample should fix that.
   */

  /* Get control register */
  digitalWrite (A0, LOW);
  delay(1);

  ulong r = ad_sample (true);

  digitalWrite (A0, HIGH);
  delay (1);

  return r;
}

void ad_configure ()
{
  /* Configure AD */
  pinMode (SDATA, OUTPUT);

  digitalWrite (A0, HIGH);
  digitalWrite (nRFS, HIGH);
  digitalWrite (nTFS, HIGH);

  /*
   * Activate 24 BIT words, self-calibration and set up notch
   * frequency.
   *
   */

  //Serial.println ("[AD7710] Configure word-length, notch frequency and activate self-calibration.. ");

  delay(100);
  digitalWrite (nTFS, LOW);
  digitalWrite (A0, LOW);
  delay (100);

  /* Configuration masks (12 bits) */

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


  // Build control configuration, total of 24 bits.
  ulong ctb = 0;

  ctb  = (ulong) (CONTROL_SELF_CALIBRATION + CONTROL_24BIT) << 12;
  ctb += (ulong) FREQUENCY;


  /*
  Serial.print ("[AD7710] Writing to control register: ");
  Serial.println (ctb, BIN);
  */


  for (int i = 2; i >= 0; i--) {

    byte b = ctb >> (8*i);
    shiftOut (SDATA, SCLK, MSBFIRST, b);
  }

  delay (1);
  digitalWrite (nTFS, HIGH);
  digitalWrite (A0, HIGH);
  delay (1);

  /* Waiting for calibration to finish */
  while (digitalRead(nDRDY)) delay(1);

  pinMode(SDATA, INPUT);

  //Serial.println ("[AD7710] Calibration finished, ready for normal operation.");
}

/*
 * Get sample from AD, will return latest value even if it has been
 * read before. Use sample (true) for a blocking version.
 */
ulong ad_sample ()
{
  ulong r = 0;

  digitalWrite (nRFS, LOW);

  // TODO: Check timing requirements from RFS HIGH to data ready.
  //       And check if that works in interrupt mode..

  /* Shift 24 bits = 3 bytes in from AD serial register */

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

/* Version of sample that waits for DRDY */
ulong ad_sample (bool blocking)
{
  if (blocking)
    while (digitalRead(nDRDY)) delayMicroseconds(1);

  return ad_sample ();
}

/* Performance test of sampling */
void ad_sample_performance_test ()
{
  Serial.println ("[AD7710] Sample performance test");
  Serial.println ("[AD7710] Timing 1000 samples..");

  ulong end = 0;
  ulong start = micros ();

  for (int i = 1001; i > 0; i--) {
    while (digitalRead(nDRDY)) ;  // wait for DRDY
    ad_sample ();                 // sample
  }

  end = micros ();
  ulong total = (ulong)(end - start) / 1000.0L;

  Serial.print ("[AD7710] Duration [ms]: ");
  Serial.println (total);
}

/* Calculate sample rate, resets counter */
ulong ad_sample_rate ()
{
  ulong r =  (ad_samples * 1000) / (millis () - ad_start);

  /* Reset sample rate counter */
  ad_samples = 0;
  ad_start   = millis ();

  return r;
}

ulong ad_get_value ()
{
  return ad_value;
}

# endif

/* vim: set filetype=arduino :  */

