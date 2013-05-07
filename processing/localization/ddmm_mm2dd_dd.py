#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-05-07
#

import os
import sys

from utils import *

for l in sys.stdin.readlines():
  [lon, lat] = l.strip().split()
  try:
    lat = ddmm_mm_decimaldegree (lat[:-1], lat[-1:])
    lon = ddmm_mm_decimaldegree (lon[:-1], lon[-1:])
  except:
    lat = 0
    lon = 0

  sys.stdout.write ('%f2     %f2\n' % (lon, lat))


