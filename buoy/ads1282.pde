/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-16
 *
 * Interface to the ADS1282.
 *
 */

# ifndef ADS1282
# define ADS1282

# include "buoy.h"
# include "ads1282.h"
# include "gps.h"
# include "store.h"

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

volatile  sample  ad_value   = {0, 0, 0}; /* Last sampled value */
          sample  ad_config  = {0, 0, 0};

volatile  ulong   ad_start = 0;       // Time last queue fill happened
volatile  ulong   ad_queue_time = 0;  // Time to fill up queue (millis)

volatile  sample  ad_queue[AD_QUEUE_LENGTH];
volatile  ulong   ad_time[AD_QUEUE_LENGTH];
volatile  uint    ad_qposition;

volatile  byte    batchready;

void ad_setup ()
{
  memset ((void *)ad_queue, 0, 3 * AD_QUEUE_LENGTH);

  ad_qposition  = 0;
  batchready    = 0;

  /* Setting up pins */
# if 0
  pinMode (nDRDY, INPUT);
  pinMode (A0, OUTPUT);
  pinMode (nRFS, OUTPUT);
  pinMode (nTFS, OUTPUT);
  pinMode (SCLK, OUTPUT);
  pinMode (SDATA, INPUT);
# endif

  ad_configure ();

  ad_read_control_register ();

  ad_start = millis ();

# ifdef AD_MONITOR_DRDY
  /* Configure interrupt */
  attachInterrupt (nDRDY_INTERRUPT, ad_drdy, LOW);
# endif
}

/* Will be run on nDRDY LOW */
void ad_drdy ()
{
  /* Check for micros () overflow */
  CHECK_FOR_OVERFLOW();

  ad_sample (); // Puts latest sample in ad_value

  /* ad_value cannot change while in this interrupt */
  memcpy ((void *)ad_queue[ad_qposition], (const void*)ad_value, 3);

  /* set time */
  ad_time[ad_qposition]  = TIME_FROM_REFERENCE;

  if (ad_qposition == AD_QUEUE_LENGTH / 2) {
    batchready = 1;
    ad_qposition++;
  } else if (ad_qposition >= AD_QUEUE_LENGTH) {
    ad_qposition = 0;
    batchready = 2;

    // Sample rate calculation
    ad_queue_time = millis () - ad_start;
    ad_start      = millis ();

  } else {
    ad_qposition++;
  }

  lastmicros = micros ();
}

void ad_loop ()
{
# if 0
  if (batchready > 0) {
    /* Send data over RF */
    rf_ad_message (AD_DATA_BATCH);

    /* Store data on SD */
    sd_write_batch ();

    batchready = 0;
  }
# endif
}

void ad_read_control_register ()
{
# if 0

# ifdef AD_MONITOR_DRDY
  detachInterrupt (nDRDY_INTERRUPT);
# endif

  /* Get control register */
  digitalWrite (A0, LOW);
  delay(1);

  ad_sample (true); // Empty output register (see above)
  ad_sample (true);

  for (int i = 0; i < 3; i ++)
    ad_config[i] = ad_value[i];

  digitalWrite (A0, HIGH);
  delay (1);

# ifdef AD_MONITOR_DRDY
  attachInterrupt (nDRDY_INTERRUPT, ad_drdy, LOW);
# endif

# endif
}

void ad_configure ()
{
# if 0
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

  delay(100);
  digitalWrite (nTFS, LOW);
  digitalWrite (A0, LOW);
  delay (100);

  // Build control configuration, total of 24 bits.
  ulong ctb = 0;

  ctb  = (ulong) AD_CONTROL << 12;
  ctb |= (ulong) FREQUENCY_CODE;

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
# endif 
}

/*
 * Get sample from AD, will return latest value even if it has been
 * read before. Use ad_sample (true) for a blocking version.
 */
void ad_sample ()
{
# if 0
  digitalWrite (nRFS, LOW);

  // TODO: Check timing requirements from RFS HIGH to data ready.
  //       And check if that works in interrupt mode..

  /* Shift 24 bits = 3 bytes in from AD serial register */

  for (int i = 0; i < 3; i++)
    ad_value[i] = shiftIn (SDATA, SCLK, MSBFIRST);

  digitalWrite (nRFS, HIGH);
# endif
}

/* Version of sample that waits for DRDY */
void ad_sample (bool blocking)
{
# if 0
  if (blocking)
    while (digitalRead(nDRDY)) delayMicroseconds(1);

  return ad_sample ();
# endif
}

# if DIRECT_SERIAL
/* Performance test of sampling */
void ad_sample_performance_test ()
{
# if 0
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
# endif
}
# endif

# endif

/* vim: set filetype=arduino :  */

