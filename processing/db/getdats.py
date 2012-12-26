#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-26
#
# getdats.py: Get DATs for mseeds
#

import os
import sys
import shutil

# stations
stations = ['GAK2', 'GAK3', 'GAK4', 'GAKS']
sdirs = { 'GAK2' : 'b2', 'GAK3' : 'b3', 'GAK4': 'b4', 'GAKS' : 's5' }

# dirs
contdir = '01_cont'
rawevents = '02_events_raw'
readyevents = '03_events_ready'

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

# mseeds
mseed = []
files = os.listdir ('./')
for f in files:
  if '.mseed' in f:
    mseed.append (f)

for f in mseed:
  s = f[f.find ('GAK'):f.find ('GAK') + 4]
  if 'S' not in s:
    i = f[:-5] + 'ids'

    if not os.path.exists (s):
      os.makedirs (s)

    print "Working on: %s" % s

    # copy dats
    fi = open (i, 'r')
    for ii in fi.readlines ():
      ii = ii.split(',')[0]
      dst = s + '/'
      src = os.path.join (root, sdirs[s], 'dat', ii + '.DAT')
      print "Copying: %s.." % src
      shutil.copy (src, dst)

      src = os.path.join (root, sdirs[s], 'dat', ii + '.IND')
      print "Copying: %s.." % src
      shutil.copy (src, dst)

    fi.close ()


