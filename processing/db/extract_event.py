#! /usr/bin/python2
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-25
#
# Extract events creates a folder for an event ready with event S file
# mseed and DAT files

import sys
import os
import os.path
import shutil

# options:
# -a extract all events
# -l list extractable events

if len(sys.argv) != 2:
  print "No argument speicfied."
  print ""
  print "extract_event.py [-a|-l] event"
  print ""
  print " -a    extract all events"
  print " -l    list all events"
  print ""
  print " event name without arguments extracts this event."
  print ""
  sys.exit (1)

extractall = ('-a' in sys.argv)
listevents = ('-l' in sys.argv)
singleevent = (not (extractall or listevents))
event = ''

if singleevent:
  event = sys.argv[1]

# stations
stations = ['GAK2', 'GAK3', 'GAK4']
sdirs = { 'GAK2' : 'b2', 'GAK3' : 'b3', 'GAK4': 'b4' }

# dirs
contdir = '01_cont'
rawevents = '02_events_raw'
readyevents = '03_events_ready'

# figure out which dir im run in: either db root or one of sub-dbs
root = './'
if not os.path.exists (os.path.join (root, contdir)):
  root = '../'
  if not os.path.exists (os.path.join (root, contdir)):
    print "Could not figure out root db dir, run from either root db or one of sub dirs."
    sys.exit (1)

contdir = os.path.join (root, contdir)
rawevents = os.path.join (root, rawevents)
readyevents = os.path.join (root, readyevents)

def extract_event (sfile):
  print "Extracting event: %s.." % sfile
  if not os.path.exists (sfile):
    print "Event file %s does not exist." % sfile
    sys.exit (1)

  # create dir for new event
  eventdir = os.path.join (readyevents, os.path.basename (sfile))
  if not os.path.exists (eventdir):
    os.makedirs (eventdir)

  # write out event with updated mseed file names (same dir)
  newsfile = os.path.join (eventdir, os.path.basename (sfile))
  newsfl = open (newsfile, 'w')

  mseedfiles = []

  sfl = open (sfile, 'r')
  for l in sfl.readlines ():
    if 'mseed' in l:
      n = l.find ('.mseed')
      m = l[:n]

      mseedfiles.append (m + '.mseed')

      newl = ' ' + os.path.basename (m) + '.mseed'
      k = len(newl)
      while k < 79:
        newl = newl + ' '
        k += 1

      newl += '6\n'

      newsfl.write (newl)

    else:
      newsfl.write (l)


  sfl.close ()
  newsfl.close ()

  # Copy mseed files
  for f in mseedfiles:
    f = f.strip ()
    shutil.copy (os.path.join (root, f[3:]), eventdir)

  # read ids and refs file for each mseed file
  for f in mseedfiles:
    f = f.strip ()
    ff = os.path.join (root, f[3:])

    indf = ff[:-5] + 'ids'
    refs = ff[:-5] + 'refs'
    shutil.copy (indf, eventdir)
    shutil.copy (refs, eventdir)

    # figure out which station
    n = ff.find ('_GAK')
    s = ff[n+4]

    # create dir for station
    sd = os.path.join (eventdir, 'GAK' + s)
    if not os.path.exists (sd):
      os.makedirs (sd)

    # copy dats to station folder
    datdir = os.path.join (root, sdirs['GAK' + s], 'dat')
    indfd = open (indf, 'r')
    for i in indfd.readlines ():
      ii = i.split(',')
      shutil.copy (os.path.join (datdir, ii[0] + '.DAT'), sd)
      shutil.copy (os.path.join (datdir, ii[0] + '.IND'), sd)
    indfd.close ()


if listevents:
  print "Listing events in: %s" % rawevents
  files = os.listdir (rawevents)
  files.sort ()

  total = 0
  for f in files:
    if '.S' in f:
      d = f[:2]
      ym = f[-6:]
      y = ym[:4]
      m = ym[5:]

      print "%d-%02d-%02d: %s" % (int(y), int(m), int(d), f)
      total += 1

  print "Total of %d events." % total


elif extractall:
  print "Extracting all events in: %s " % rawevents
  files = os.listdir (rawevents)
  files.sort ()

  total = 0
  for f in files:
    if '.S' in f:
      extract_event (f)

      total += 1

  print "Extracted a total of %d events." % total

elif singleevent:
  extract_event (event)
  sys.exit (0)

else:
  # should not happen
  sys.exit (1)





