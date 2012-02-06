/* Arduino SdFat Library
 * Copyright (C) 2008 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SdFatUtil_h
#define SdFatUtil_h
/**
 * \file
 * \brief Useful utility functions.
 */
#include <WProgram.h>
# if DIRECT_SERIAL
/** Store and print a string in flash memory.*/
#define PgmPrint(x) SerialUSB.print(x)
/** Store and print a string in flash memory followed by a CR/LF.*/
#define PgmPrintln(x) SerialUSB.println(x)
# else
/** Store and print a string in flash memory.*/
#define PgmPrint(x) 
/** Store and print a string in flash memory followed by a CR/LF.*/
#define PgmPrintln(x)

# endif

namespace SdFatUtil {
  int FreeRam();
}

using namespace SdFatUtil;  // NOLINT
#endif  // #define SdFatUtil_h
