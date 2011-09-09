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

extern volatile ulong ad_value;

void ad_setup ();

ulong ad_get_value ();
ulong ad_sample_rate ();

# endif

