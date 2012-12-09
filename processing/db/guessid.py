#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-09
#
# guessid.py: Guess which ID file event starts at, run from event dir.

import os
import os.path
import sys
import datetime
import time

stations = ['GAK2', 'GAK3', 'GAK4']

event = os.path.basename (os.getcwd ())
date = event[-6:] + event[:2]

print "Event: %s" % event

if not '.S' in event:
  print "Error: Does not seem like event dir."
  sys.exit (1)

if not os.path.exists (event):
  print "Error: No event file."
  sys.exit (1)

ef = open (event, 'r')
lines = ef.readlines ()
ef.close ()

# Find mseed files with refs and positions
mseedfiles = []
for l in lines:
  if 'mseed' in l:
    mseedfiles.append (l[:l.find('.mseed')].strip ())

reffiles = dict()
for m in mseedfiles:
  for s in stations:
    if s in m:
      reffiles[s] = m + '.refs'

phases = []
for l in lines:
  if 'IP' in l and l[79] == ' ':
    station = l[1:5].strip ()
    phasen  = l[9:15].strip ()
    hrmm    = l[18:22].replace (' ', '0').strip ()
    secs    = l[23:28].replace (' ', '0').strip ()
    print "=> Station: %s, phase: %s, time: %s:%s" % (station, phasen, hrmm, secs)
    phases.append ([station, phasen, hrmm, secs])

print
print "Guesses:"

for s in stations:
  found = False
  for p in phases:
    if s == p[0] and p[1] == 'IP':
      found = True
      times = date + p[2] + p[3][:-3] + ' UTC'
      d = datetime.datetime.strptime (times, '%Y%m%d%H%M%S %Z')
      ts = (d - datetime.datetime (1970, 1, 1)).total_seconds ()
      ts += float(p[3][-3:])
      hts = ts * 1e6

      #print "=> %s, first arrival (%s): %s, timestamp: %f" % (s, p[1], times, ts)

      # Searching for closest position in ref file
      prev = ''
      next = ''

      rlf = open (reffiles[s], 'r')
      for rl in rlf:
        f = rl.split (',')
        if int(f[2]) >= hts:
          next = rl.strip ()
          break
        elif int(f[2]) <= hts:
          prev = rl.strip ()

      if next == '' or prev == '':
        print "=> Could not find fitting reference in %s file (reference required both before and after IP pick), exiting." % reffiles[s]
        sys.exit (1)

      prevf = next.split (',')
      nextf = next.split (',')

      print '=> %s: Closest ID to IP phase: %5d' % (s, int(prevf[0]))

      break

  if not found:
    print "=> No IP phase for station: %s, exiting." % s
    sys.exit (1)
