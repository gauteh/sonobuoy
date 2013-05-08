#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-05-07
#

import os
import sys

from utils import *

station = sys.argv[1]
if station == 'B1':
  no = 3
elif station == 'B2':
  no = 0
elif station == 'B3':
  no = 1
elif station == 'B4':
  no = 2
else:
  print "Unknown station"
  sys.exit (1)


for l in sys.stdin.readlines():
  try:
    [lon, lat] = l.strip().split()
    lat = ddmm_mm_decimaldegree (lat[:-1], lat[-1:])
    lon = ddmm_mm_decimaldegree (lon[:-1], lon[-1:])
  except:
    lat = 0
    lon = 0

  sys.stdout.write ('%f2     %f2   %d\n' % (lon, lat, no))


