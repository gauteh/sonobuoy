/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Buoy controller header, definitions and conventions.
 *
 */

# ifndef BUOY_H
# define BUOY_H

/*
# define VERSION_BASE "0.1.0"

# ifdef GIT_SHA
  # define VERSION VERSION_BASE " " GIT_SHA
# else
  # define VERSION VERSION_BASE
# endif
*/

# define VERSION GIT_SHA

/* Define to include functionality that expects a terminal connected
 * to Serial0. */
# define DIRECT_SERIAL 0

# define GREETING "Gautebøye ( rev " VERSION " ) by " \
                  "Gaute Hope <eg@gaute.vetsj.com>"

# define ULONG_MAX 4294967295u // 2^32 - 1

/* Architectures:
 *
 * AVR:
 * uint32_t ulong;
 * uint16_t uint;
 *
 * PC:
 * uint64_t ulong;
 * uint32_t uint;
 *
 * TODO: Get rid of ulong and uint and replace by exact size!!!
 */
typedef unsigned long ulong;
typedef unsigned int  uint;

# endif

/* vim: set filetype=arduino :  */

