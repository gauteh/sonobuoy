#! /usr/bin/python2
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-02-18
#
# Read phases and output to stdout


import sys
import os, stat
import os.path
import datetime
import time
from shutil import copy
from utils import *
import math

stations = ['GAK2', 'GAK3', 'GAK4']
seismo   = 'GAKS'

if len(sys.argv) == 2:
  event = sys.argv[1]
else:
  event = os.path.basename(os.getcwd()) # default

if not os.path.exists (event):
  print "Could not open event file: %s.." % event
  sys.exit (1)


datadir = os.path.join (os.path.dirname (sys.argv[0]))

date = event[-6:] + event[:2]

s = open (event, 'r')
lines = s.readlines ()
s.close ()

# Read phases from S file
phaselines = []
for l in lines:
  if ('IP' in l or 'IS' in l or 'IPn' in l or 'EPn' in l) and (l[79] == ' '):
    phaselines.append (l)

# Parse phases
# format: [station, phasename, hrmm, second]
phases = []
for l in phaselines:
  station = l[1:5].strip ()
  phasen  = l[9:15].strip ()
  hrmm    = l[18:22].replace (' ', '0').strip ()
  secs    = l[23:28].replace (' ', '0').strip ()
  phases.append ([station, phasen, hrmm, secs])


correctedphases = []
for p in phases:
  if p[1] == 'IP':
    p[1] = 'P_'
  elif p[1] == 'IS':
    p[1] = 'S_'

  elif p[1] == 'IPn':
    p[1] = 'M_'

  elif p[1] == 'EPn':
    p[1] = 'MM'

  correctedphases.append (p)

correctedphases.sort ()  # IP/P must come before IS/S

for p in correctedphases:
  sys.stdout.write ('{station:<5s} {phasename:8s} {year:4d} {month:02d} {day:02d} {hour:02d} {minute:02d} {second:06.3f} {steering:6s}\n'.format (

        station       = p[0],
        phasename     = p[1],
        year          = int(date[0:4]),
        month         = int(date[5:6]),
        day           = int(date[7:]),
        hour          = int(p[2][:2]),
        minute        = int(p[2][2:]),
        second        = float(p[3]),
        steering      = 'T__D__'

      ))




