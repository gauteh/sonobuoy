#! /usr/bin/python
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-27
#
# Utility functions

import math

def decimal_degree (s, d):
  # s is decimalminute.fractionalminutes string
  # d is north/south or east/west indicator

  minu = float (s[s.find('.') -2:])
  deg  = float (s[:s.find ('.') - 2])

  deg += minu / 60.0

  if d == 'S' or d == 'W':
    deg *= -1.0

  return deg

def decimaldegree_ddmmss (s):
  deg   = math.floor (s)
  mint  = (s - deg) * 60.0
  min   = math.floor (mint)
  sec   = (mint - min) * 60.0

  r = '%02d%02d%04.1f' % (deg, min, sec)

  return r

def ddmmss_decimaldegree (s):
  s = s.replace (' ', '0') # space -> 0

  n = s.find ('.')
  sec = s[n -2:-1]
  d   = s[-1]
  min = s[n - 4: n - 2]
  deg = s[: n - 4]

  r  = float (deg)
  r += float (min) / 60.0
  r += float (sec) / 60.0 / 60.0

  if d == 'S' or d == 'W':
    r *= -1.0

  return r

