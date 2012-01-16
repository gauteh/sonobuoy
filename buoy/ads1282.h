/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-16
 *
 * Interface to the ADS1282.
 *
 */

# ifndef ADS1282_H
# define ADS1282_H

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


/* Configuration mask to be set */

# define FREQUENCY 250 

/* Sample is 24 bit / 8 = 3 bytes */
typedef byte    sample[3];

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
//# define AD_MONITOR_DRDY

# endif

/* vim: set filetype=arduino :  */

