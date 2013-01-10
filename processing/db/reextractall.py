#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-07-08
#
# Run through all previously extracted events in 03_events_ready,
# extract event and putrevised.
#
# Warning: Will overwrite exiting events in 04.
#

import os
import sys
import os.path
import shutil
import datetime

sys.path.append (os.path.join (os.path.dirname (os.path.realpath(__file__)), '../../zero/storetools/python'))

from makemseed  import Makemseed
from putrevised import Putrevised


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

# Don't work on events with some sort of comment
# Determine if event is ready:
#   1. no comment
#   2. has mseed with ids, refs in all station folders

# Back up previous solution to 00/ sub-folder in station folder


print "Reextracting all previously revised events in 03_events_ready.."
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


  # check if all stations have been extracted
  hasallstations = True

  idfiles = {} # station <-> idfile dict

  for s in allstations:
    sdir = os.path.join (d, s)

    if not os.path.exists (os.path.join (sdir, '00')):
      os.makedirs (os.path.join (sdir, '00')) # make backup dir

    if not os.path.exists (os.path.join (sdir, '01')):
      os.makedirs (os.path.join (sdir, '01')) # make new dir

    # check if already backed up
    backedup = False
    if os.path.exists (os.path.join (sdir, '00', 'reextract_backed_up')):
      backedup = True
      sdir = os.path.join (sdir, '00')

    hasmseed = False
    hasids   = ('GAKS' in s)
    hasrefs  = ('GAKS' in s)

    stafiles = os.listdir (sdir)
    for f in stafiles:
      copy = False

      if 'mseed' in f:
        hasmseed = True
        copy = True

      if 'refs' in f:
        hasrefs = True
        copy = True

      if 'ids' in f:
        hasids = True
        copy = True
        idfiles[s] = f

      if copy:
        orig = os.path.join (sdir, f)
        if backedup:
          print "%s: already backed up: %s" % (d, f)
        else:
          print "%s: backing up %s.." % (d, f)
          dest = os.path.join (sdir, '00/')
          shutil.copy (orig, dest)

        # if seismo, copy to new as well
        if 'GAKS' in s:
          print "%s: copying to new %s.." % (d, f)
          if backedup:
            dest = os.path.join (sdir, '..', '01/')
          else:
            dest = os.path.join (sdir, '01/')

          shutil.copy (orig, dest)

        if not backedup:
          print "%s: deleting %s" % (d, orig)
          os.remove (orig)


    if hasmseed and hasids and hasrefs:
      hasallstations &= True
      bdf = open (os.path.join (d, s, '00', 'reextract_backed_up'), 'w')
      bdf.write ('yes\n')
      bdf.close ()
    else:
      hasallstations = False

  if hasallstations:
    print "%s: reextracting.." % d
    for s in stations:
      reextract_done = False
      if os.path.exists (os.path.join (d, s, '01', 'reextract_done')):
        reextract_done = True

      if not reextract_done:
        idf = open (os.path.join (d, s, '00', idfiles[s]), 'r')
        ids = []
        for l in idf.readlines ():
          (ii, sdlag) = l.split (',')
          ids.append (int(ii))

        idf.close ()

        mkm = Makemseed ()
        mkm.station = s
        mkm.root = os.path.join (d, s)
        mkm.destdir = os.path.join (d, s, '01')
        mkm.ids = ids
        (mseedf, idsf, refsf) = mkm.dorange ()

        rd = open (os.path.join (d, s, '01', 'reextract_done'), 'w')
        rd.write ('yes\n')
        rd.close ()
      else:
        print "%s: %s already reextracted, skipping." % (d, s)

    print "%s: putting to 04_events_localize.." % d
    p = Putrevised ()
    p.doevent (d, d, '01')

  else:
    print "%s: not fully revised. ignoring." % d


