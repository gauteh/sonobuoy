#! /usr/bin/python2
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-26
#
# Extract revised mseeds and create new S file ready for phasepicking
#
# Run from event dir in 03_events_ready
#
# usage:
#   putrevised.py [subfolder]
#
#   subfolder       sub-folder below station folder with extracted data
#


import sys
import os
import os.path
import shutil
import datetime

# stations
stations = ['GAK2', 'GAK3', 'GAK4']
seismos  = 'GAKS'
sdirs = { 'GAK2' : 'b2', 'GAK3' : 'b3', 'GAK4': 'b4' }

# dirs
contdir = '01_cont'
rawevents = '02_events_raw'
readyevents = '03_events_ready'
localizeevents = '04_events_localize'

root = '../../'
if not os.path.exists (os.path.join (root, contdir)):
  root = '../'
  if not os.path.exists (os.path.join (root, contdir)):
    print "Could not figure out root db dir, run from event dir in 03_events_ready/_some_event_ ."
    sys.exit (1)

contdir = os.path.join (root, contdir)
rawevents = os.path.join (root, rawevents)
readyevents = os.path.join (root, readyevents)
localizeevents = os.path.join (root, localizeevents)

class Putrevised:
  mseedfiles = None
  eventroot  = ''
  neweventd  = ''

  def __init__ (self):
    self.mseedfiles = []

  def reset (self):
    self.mseedfiles = []

  def find_copy_wavs (self, stationsub):
    if stationsub is None:
      stationsub = '.'

    for s in stations:
      sd = os.path.join (self.eventroot, s, stationsub)
      files = os.listdir (sd)
      for f in files:
        if '.ids' in f or '.refs' in f or '.mseed' in f:
          print "Copying: %s.." % os.path.join (sd, f)
          shutil.copy (os.path.join (sd, f), self.neweventd)

          if f[-6:] == '.mseed':
            self.mseedfiles.append (f)

    # copy seismometer mseeds
    sd = os.path.join (self.eventroot, 'GAKS', stationsub)
    files = os.listdir (sd)
    for f in files:
      if 'GAKS' in f:
        print "Copying: %s.." % os.path.join (sd, f)
        shutil.copy (os.path.join (sd, f), self.neweventd)
        self.mseedfiles.append (f)

  def doevent (self, eventroot, event, stationsub = None):
    # stationsub is subfolder inside station folder where extracted wave
    # file is located
    self.eventroot = eventroot

    self.neweventd = os.path.join (localizeevents, event)
    if not os.path.exists (self.neweventd):
      os.makedirs (self.neweventd)


    self.find_copy_wavs (stationsub)

    # generate S file
    neweventf = os.path.join (self.neweventd, event)
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
    for f in self.mseedfiles:
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

if __name__ == '__main__':
  # i'm run from event dir
  c = os.getcwd ()
  event = os.path.basename (c)
  eventroot = '.'

  subfolder = None
  if len(sys.argv) == 2:
    subfolder = sys.argv[1]

  p = Putrevised ()
  p.doevent (eventroot, event, stationsub = subfolder)

