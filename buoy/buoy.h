/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-03
 *
 * Buoy controller header, definitions and conventions.
 *
 */

# ifndef BUOY_H
# define BUOY_H

# define VERSION_BASE "0.1.0"
# ifdef GIT_SHA
# define VERSION VERSION_BASE " " GIT_SHA
# else
# define VERSION VERSION_BASE
# endif


# define GREETING \
"Buoy Control ( version " VERSION " )\n" \
"by Gaute Hope <eg@gaute.vetsj.com> / <gaute.hope@student.uib.no>  (2011)"

typedef unsigned long ulong;
typedef unsigned int  uint;


# endif

/* vim: set filetype=arduino :  */

