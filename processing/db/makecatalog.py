#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> /  2012-12-03
#
# List extracted events and list unexctracted events with comments.


import os
import os.path
import sys
import shutil

sys.path.append (os.path.join (os.path.dirname (os.path.realpath(__file__)), '../../zero/storetools/python'))

from guessid    import Guessid

# stations
stations = ['GAK2', 'GAK3', 'GAK4', 'GAKS']
sdirs = { 'GAK2' : 'b2', 'GAK3' : 'b3', 'GAK4': 'b4', 'GAKS' : 's5' }

# dirs
contdir = '01_cont'
rawevents = '02_events_raw'
readyevents = '03_events_ready'
localizeevents = '04_events_localize'

# figure out which dir im run in: either db root or one of sub-dbs
root = './'
if not os.path.exists (os.path.join (root, contdir)):
  root = '../'
  if not os.path.exists (os.path.join (root, contdir)):
    root = '../../'
    if not os.path.exists (os.path.join (root, contdir)):
      print "Could not figure out root db dir, run from either root db or one of sub dirs."
      sys.exit (1)

contdir = os.path.join (root, contdir)
rawevents = os.path.join (root, rawevents)
readyevents = os.path.join (root, readyevents)
localizeevents = os.path.join (root, localizeevents)


# Search for events in 03
raw = []

files = os.listdir ('.')
files.sort ()
for f in files:
  if '.S' in f:
    raw.append (f)

# print events as they arrive at GAK2

for e in raw:
  g = Guessid ()
  ids = g.go (e, e, quiet = True)

  print ids['GAKS'][1]
  

