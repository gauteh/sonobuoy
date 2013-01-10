#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-07-08
#
# Extract all un-extracted events to subdirs '02' (under station)
#
# Usage:
#   autoextract.py [-p]
#
#   -p    put extracted events (or previously autoextracted events)
#
#         Warning: May overwrite existing events in 04
#

import os
import sys
import os.path
import shutil
import datetime

import subprocess

sys.path.append (os.path.join (os.path.dirname (os.path.realpath(__file__)), '../../zero/storetools/python'))

from makemseed  import Makemseed
from putrevised import Putrevised
from guessid    import Guessid

putautoextracted = False
if '-p' in sys.argv:
  putautoextracted = True


# stations
stations = ['GAK2', 'GAK3', 'GAK4']
seismos  = 'GAKS'
allstations = ['GAK2', 'GAK3', 'GAK4', 'GAKS']
sdirs = { 'GAK2' : 'b2', 'GAK3' : 'b3', 'GAK4': 'b4' }

# dirs
contdir = '01_cont'
rawevents = '02_events_raw'
readyevents = '03_events_ready'
localizeevents = '04_events_localize'

root = '../'
if not os.path.exists (os.path.join (root, contdir)):
  print "Could not figure out root db dir, run from event dir in 03_events_ready ."
  sys.exit (1)

contdir = os.path.join (root, contdir)
rawevents = os.path.join (root, rawevents)
readyevents = os.path.join (root, readyevents)
localizeevents = os.path.join (root, localizeevents)

# Do not work on events with some sort of comment
# Do not work on events that already have been extracted
#
# Determine if event is ready:
#   1. no comment
#   2. has not already been extracted

# Back up previous solution to 00/ sub-folder in station folder


print "Auto extracting all unhandled events in 03_events_ready.."
events_ready = os.listdir (readyevents)

events_ready.sort ()
for d in events_ready:
  print "%s: examining.." % d
  # check for comment
  commentf = os.path.join (d, d + '.txt')

  if os.path.exists (commentf):
    commentf = open (commentf, 'r')
    comment = commentf.read ()
    comment = comment.strip ()
    print "%s: Ignoring, reason: comment: %s" %(d, comment)
    continue

  # check if event has already been extracted / revised
  alreadyextracted = True

  for s in allstations:
    sdir = os.path.join (d, s)

    ex_00 = os.path.join (sdir, '00', 'reextract_backed_up')
    ex_01 = os.path.join (sdir, '01', 'reextract_done')

    if (os.path.exists (ex_00) or os.path.exists (ex_01)):
      alreadyextracted &= True
    else:
      alreadyextracted = False

  if alreadyextracted:
    print "%s: already extracted, ignoring." % d
    continue

  else:
    print "%s: extracting.." % d

    # guess id
    g = Guessid ()
    ids = g.go (d, d, quiet = True)

    marginids = 1 # id files to include before and after guessed id
    marginseismo_pre  = 30.0  # seconds before event to include on seismometer
    marginseismo_post = 160.0 # seconds to include after first phase arrival

    # user arrival on GAK2 as arrival for GAKS

    for s in stations:
      sdir = os.path.join (d, s)
      ex_02 = os.path.join (sdir, '02')
      if not os.path.exists (ex_02):
        os.makedirs (ex_02)

      extract_done = False
      if os.path.exists (os.path.join (ex_02, 'extract_done')):
        extract_done = True

      if not extract_done:
        mids = []

        for i in range (ids[s][0] - marginids, ids[s][0] + marginids + 1):
          if os.path.exists (os.path.join (sdir, str(i) + '.DAT')):
            mids.append (i)

        # extract to 02
        mkm = Makemseed ()
        mkm.station = s
        mkm.root = os.path.join (d, s)
        mkm.destdir = ex_02
        mkm.ids = mids
        (mseedf, idsf, refsf) = mkm.dorange ()

        rd = open (os.path.join (ex_02, 'extract_done'), 'w')
        rd.write ('yes\n')
        rd.close ()

      else:
        print "%s: %s already extracted, skipping station." % (d, s)

    # seismometer
    s = 'GAKS'
    sdir = os.path.join (d, s)
    ex_02 = os.path.join (sdir, '02')
    if not os.path.exists (ex_02):
      os.makedirs (ex_02)

    extract_done = False
    if os.path.exists (os.path.join (ex_02, 'extract_done')):
      extract_done = True

    if not extract_done:
      mids = []

      # extract to 02
      start = ids['GAK2'][1] - datetime.timedelta(seconds = marginseismo_pre)
      start_cmd = start.strftime ('%H%M%S.0')

      cutseismo = os.path.dirname (os.path.realpath (__file__))
      cutseismo = os.path.join (cutseismo, 'cutseismo.sh')
      cmd = '%s %s %d' % (cutseismo, start_cmd, marginseismo_post)
      print "%s: GAKS: executing %s.." % (d, cmd)

      p = subprocess.Popen ([cutseismo, start_cmd, str(marginseismo_post)], cwd = d, stdout = subprocess.PIPE)
      p.wait ()
      out = p.stdout.read ()

      if p.returncode != 0:
        print "%s: GAKS: Failed to extract seismometer wavefile, skipping." % d
        continue 

      n = out.find ('file to: ')
      mseedf = out[n+9:-2]
      print "%s: GAKS: Moving seismometer wav to 02..: %s" % (d, mseedf)
      if os.path.exists (os.path.join (ex_02, mseedf)):
        print "%s: GAKS: mseed exists, deleting old." % d
        os.remove (os.path.join (ex_02, mseedf))

      shutil.move (os.path.join (sdir, mseedf), ex_02)

      rd = open (os.path.join (ex_02, 'extract_done'), 'w')
      rd.write ('yes\n')
      rd.close ()

    else:
      print "%s: %s already extracted, skipping station." % (d, s)

    if putautoextracted:
      print "%s: putting to 04_events_localize.." % d
      p = Putrevised ()
      p.doevent (d, d, '02')


