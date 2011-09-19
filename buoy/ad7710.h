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

/* Sample is 24 bit / 8 = 3 bytes */
typedef byte sample[3];

extern volatile sample ad_value;

# define AD_QUEUE_LENGTH 500
extern volatile uint    ad_qposition;
extern volatile sample  ad_queue[];
extern volatile ulong   ad_time[];

void          ad_setup ();
void          ad_configure ();
void          ad_sample ();
void          ad_sample (bool);
ulong         ad_sample_rate ();
void          ad_read_control_register ();

/* Turn on DRDY interrupt and sample AD is ready  */
# define AD_MONITOR_DRDY

# endif

/* vim: set filetype=arduino :  */

