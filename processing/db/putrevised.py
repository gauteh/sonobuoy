#! /usr/bin/python2
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-25
#
# Extract revised mseeds and create new S file ready for phasepicking
#
# mseed: any mseeds in station subdirs
#
# Run from event dir in 03_events_ready


import sys
import os
import os.path
import shutil
import datetime

# stations
stations = ['GAK2', 'GAK3', 'GAK4']
sdirs = { 'GAK2' : 'b2', 'GAK3' : 'b3', 'GAK4': 'b4' }

# dirs
contdir = '01_cont'
rawevents = '02_events_raw'
readyevents = '03_events_ready'
localizeevents = '04_events_localize'

root = '../../'
if not os.path.exists (os.path.join (root, contdir)):
  print "Could not figure out root db dir, run from event dir in 03_events_ready/... ."
  sys.exit (1)

contdir = os.path.join (root, contdir)
rawevents = os.path.join (root, rawevents)
readyevents = os.path.join (root, readyevents)
localizeevents = os.path.join (root, localizeevents)

# i'm run from event dir
c = os.getcwd ()
event = os.path.basename (c)

neweventd = os.path.join (localizeevents, event)
if not os.path.exists (neweventd):
  os.makedirs (neweventd)

mseedfiles = []

for s in stations:
  files = os.listdir (s)
  for f in files:
    if f[-6:] == '.mseed':
      print "Copying: %s.." % f
      shutil.copy (os.path.join (s, f), neweventd)
      mseedfiles.append (f)

# generate S file
neweventf = os.path.join (neweventd, event)
nfl = open (neweventf, 'w')

es = event
y  = es[-6:-2]
m  = es[-2:]
d  = es[:2]
t  = es[3:7]
s  = es[8:9]
dist = es[10]

print "Writing new event file: %s.." % event

# write date and location
nfl.write (" %(year)d %(month)2d%(day)2d %(time)4d %(sec)4.1f %(dist)s %(id)57d\n" % { 'year' : int(y), 'month' : int(m), 'day' : int(d), 'time' : int(t), 'sec' : int(s), 'dist' : dist, 'id': 1})

# write id field
idf = ' ACTION:ARG ' + datetime.datetime.strftime(datetime.datetime.now (), '%y-%m-%d %H:%M')
idf += ' OP:au   STATUS:               '
idf += 'ID:%(year)d%(month)02d%(day)02d%(time)4d%(sec)02d     I' % { 'year' : int(y), 'month' : int(m), 'day' : int(d), 'time' : int(t), 'sec' : int(s) }
nfl.write (idf + '\n')

# write mseed lines
for f in mseedfiles:
  nfl.write (' ' + f)
  k = len(f)+1
  while k < 79:
    nfl.write (' ')
    k += 1

  nfl.write ('6\n')

# write start of phases line
nfl.write (' STAT SP IPHASW D HRMM SECON CODA AMPLIT PERI AZIMU VELO AIN AR TRES W  DIS CAZ7\n')

# write blank line
nfl.write ('%80s' % ' ')

nfl.close ()

