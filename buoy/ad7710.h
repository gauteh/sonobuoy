/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-08
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

# ifndef AD7710_H
# define AD7710_H

# include "buoy.h"

/* Early tests: (for ulongs)
 *
 * Seems there's space for just above 1000 ulongs, meaning a
 * queue length of 500 for both time and value.
 *
 * ulong = 4 bytes, total SRAM on Mega2560 = 8K
 *
 * There is 4K available on the EEPROM
 *
 */

/* Configuration masks (12 bits) */

# define CONTROL_SELF_CALIBRATION   0b001000000000
# define CONTROL_24BIT              0b000000001000
# define CONTROL_UNIPOLAR           0b000000000001
# define CONTROL_DEFAULT            0b0

/* Configuration mask to be set */
# define AD_CONTROL (CONTROL_SELF_CALIBRATION | CONTROL_24BIT | CONTROL_UNIPOLAR)

/* Notch frequency (12 bits)
 *
 * Range: 19 - 2000uL
 * 19   gives approximately 1000 samples / 970 ms
 * 2000 gives approximately 1000 samples / 50 s
 */
# define FREQUENCY_CODE 100uL


/* F_CLK is determined by crystal, where a 10MHz should be used.
 *
 * The formula for FREQUENCY is taken from AD7710 data sheet, unsure of
 * accuracy, but it gives an estimate for making sane limitations on file
 * sizes and contents. */
# define F_CLK 10000000uL
# define FREQUENCY (F_CLK / 512) / FREQUENCY_CODE

/* Sample is 24 bit / 8 = 3 bytes */
typedef byte sample[3];

extern volatile sample ad_value;
extern          sample ad_config;

# define AD_QUEUE_LENGTH 500
extern volatile byte    batchready;
extern volatile ulong   ad_queue_time;
extern volatile uint    ad_qposition;
extern volatile sample  ad_queue[];
extern volatile ulong   ad_time[];

void          ad_setup ();
void          ad_configure ();
void          ad_sample ();
void          ad_sample (bool);
void          ad_read_control_register ();

# if DIRECT_SERIAL
void          ad_sample_performance_test ();
# endif

/* Turn on DRDY interrupt and sample AD is ready  */
# define AD_MONITOR_DRDY

# endif

/* vim: set filetype=arduino :  */

