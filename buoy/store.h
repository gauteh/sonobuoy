/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-10-06
 *
 * Handle local SD card storage
 *
 */

# ifndef STORE_H
# define STORE_H

# include <SdFat.h>

# define SD_SS    10
# define SD_MOSI  11
# define SD_MISO  12
# define SD_SCK   13

extern SdFat sd;
extern char sd_error[80];

void sd_setup ();
void sd_loop ();


# endif

/* vim: set filetype=arduino :  */

