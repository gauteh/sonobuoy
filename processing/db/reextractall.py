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

  for s in allstations:
    sdir = os.path.join (d, s)

    #if not os.path.exists (os.path.join (sdir, '00')):
      #os.makedirs (os.path.join (sdir, '00')) # make backup dir

    hasmseed = False
    hasids   = False or ('GAKS' in s)
    hasrefs  = False or ('GAKS' in s)

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

      if copy:
        print "%s: backing up %s.." % (d, f)
        orig = os.path.join (sdir, f)
        dest = os.path.join (sdir, '00/')
        #shutil.copy (orig, dest)

    if hasmseed and hasids and hasrefs:
      hasallstations &= True
    else:
      hasallstations = False

  if hasallstations:
    print "%s: putting to 04_events_localize.." % d
  else:
    print "%s: not fully revised. ignoring." % d


