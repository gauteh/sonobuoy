#! /usr/bin/python2
# 
# Test algorithm in gps.cpp:update_second
#
# Gaute Hope <eg@gaute.vetsj.com> 2012-08-13

from math import *
import time

# input
year  = 12
month =  8
day   = 13

hour    = 21
minute  = 19
second  = 59

seconds_per_day = 86400

def leap_year (x):
  return (not ((1970 + x) % 4) and ( ((1970 + x) % 100) or not ((1970 + x) % 400) ))

def leap_years_before_1970 ():
  return ((1970 / 4) - (1970 / 100) + (1970 / 400))

_year = (2000 + year) - 1970
newsecond = _year * 365 * seconds_per_day

newsecond += (( (1970 + (_year-1)) /   4 ) 
           - (  (1970 + (_year-1)) / 100 )
           + (  (1970 + (_year-1)) / 400 )
           - leap_years_before_1970 ()     ) * seconds_per_day

monthdays = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

for i in range(1, month):
  if i == 2 and leap_year (_year):
    newsecond += seconds_per_day * 29
  else:
    newsecond += seconds_per_day * monthdays[i-1]

print "Year:", newsecond
print "Bits:", log(newsecond, 2)

newsecond += seconds_per_day * (day - 1)
newsecond += hour * 60 * 60
newsecond += minute * 60
newsecond += second


print "Time:", newsecond
print "Bits:", log(newsecond, 2)
print "Bits needed:", newsecond.bit_length ()

print
print "Datetime:", time.asctime(time.gmtime(newsecond))
print "Bits needed:", newsecond.bit_length ()

