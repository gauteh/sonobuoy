/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2011-09-05
 *
 * Utility functions
 *
 */

# ifndef UTIL_C
# define UTIL_C

/* Based on Wiring's Print::printFloat (..) 
 *
 * digits < 0  means print all digits
 * */
# define MAX_FLOAT_DIGITS 32
int ftoa (char* dst, float number, uint8_t digits)
{
  int i = 0;

  // Negative number
  if (number < 0.0)
  {
    dst[i] = '-';
    number = -number;
  }

  // Round correctly
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract integer part
  unsigned long int_part = (unsigned long) number;
  double remainder = number - (double) int_part;

  i = i + sprintf(&(dst[i]), "%d", int_part);

  // Print decimal point
  if (digits != 0) {
    dst[i] = '.';
    i++;
  }

  // Extract digits from the remainder one at the time

  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toAdd = (int)remainder;
    dst[i] = (char)('0' + toAdd);
    remainder -= toAdd;
    i++;
  }

  dst[i] = 0; // NULL terminate, do not include in total count

  return (i-1);
}

# endif

