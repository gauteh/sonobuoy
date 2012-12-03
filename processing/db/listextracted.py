#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> /  2012-12-03
#
# List extracted events and list unexctracted events with comments.


import os
import os.path
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
    print "Could not figure out root db dir, run from either root db or one of sub dirs."
    sys.exit (1)

contdir = os.path.join (root, contdir)
rawevents = os.path.join (root, rawevents)
readyevents = os.path.join (root, readyevents)


# Search for events in 02
raw = []

files = os.listdir (rawevents)
files.sort ()
for f in files:
  if '.S' in f:
    if not 'txt' in f:
      raw.append (f)


# Search for events in 03
ready = []
files = os.listdir (readyevents)
files.sort ()
for f in files:
  if '.S' in f:
    ready.append (f)

# Compare
both = []
onlyraw = []
onlyready = []

for r in raw:
  if r in ready:
    both.append (r)
    ready.remove(r)
  else:
    onlyraw.append (r)

onlyready = ready

print "Events in both:"
for e in both:
  print "%s" % e

print "Number: %d" % len(both)
print

print "Events only in raw:"
withexp = 0
for e in onlyraw:
  # check for explanation
  explanation = None

  inf = os.path.join (rawevents, e + '.txt')
  if os.path.exists (inf):
    inf = open (inf, 'r')
    explanation = inf.read ()
    inf.close ()
    explanation = explanation.strip ()

  if explanation is not None:
    print "%s: %s" % (e, explanation)
    withexp += 1
  else:
    print e
  
print "Number: %d (%d without explanation)" % ( len(onlyraw), len(onlyraw) - withexp)
print

print "Events only in ready:"
withexp = 0
for e in onlyready:
  # check for explanation
  explanation = None

  inf = os.path.join (readyevents, e, e + '.txt')
  if os.path.exists (inf):
    inf = open (inf, 'r')
    explanation = inf.read ()
    inf.close ()
    explanation = explanation.strip ()
    withexp += 1

  if explanation is not None:
    print "%s: %s" % (e, explanation)
  else:
    print e
print "Number: %d (%d without explanation)" % (len(onlyready), len(onlyready) - withexp)
print
print "Total: %d" % (len(both) + len(onlyraw) + len(onlyready))


