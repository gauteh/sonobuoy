#! /usr/bin/python2
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-10-07
#
# Prepare HYPOCENTER job, run from (readily picked) event dir or with S file
# as first argument.
#
# Usage:
#
#   makecjob.py [event] [-j jobid]
#
#   event       event to create hyposat job for, if not specified
#               use current directory.
#   -j jobid    create job with id jobid in subdirectory jobid
#
# Input needed for HYPOSAT:
# - Job file: .IN (phase picks, etc) : traveltime corrected for waterdepth
# - Parameter file
# - Station coordinates
# - Regional model
# - as well as some general velocity models and data files

import sys
import os, stat
import os.path
import datetime
import time
from shutil import copy
from utils import *
import math

stations = ['GAK2', 'GAK3', 'GAK4']
seismo   = 'GAKS'

jobid = '01' # default

if '-j' in sys.argv:
  if len(sys.argv) >= 3:
    n = sys.argv.index ('-j')
    jobid = sys.argv[n+1]

    sys.argv.remove ('-j')
    sys.argv.remove (jobid)

  else:
    print "Incorrect arguments."
    sys.exit (1)

if len(sys.argv) == 2:
  event = sys.argv[1]
else:
  event = os.path.basename(os.getcwd()) # default

if not os.path.exists (event):
  print "Could not open event file: %s.." % event
  sys.exit (1)

jobd = 'job_hc%s' % jobid
if not os.path.exists (jobd):
  os.makedirs (jobd)

datadir = os.path.join (os.path.dirname (sys.argv[0]), 'hyp')

date = event[-6:] + event[:2]

print "Creating HYPOCENTER job %s for: %s in %s.. (date: %s)" % (jobid, event, jobd, date)
# filter out phases that won't be used by hypocenter
eventf = open (event, 'r')
neweventf = open (os.path.join (jobd, event), 'w')
for l in eventf.readlines ():
  if (not 'EP' in l) and (not 'IPg' in l):
    neweventf.write (l)

eventf.close ()
neweventf.close ()



s = open (event, 'r')
lines = s.readlines ()
s.close ()

print "Reading phases.."
# Read phases from S file
phaselines = []
for l in lines:
  if ('IP' in l or 'IS' in l) and (l[79] == ' '):
    phaselines.append (l)

# Parse phases
# format: [station, phasename, hrmm, second]
phases = []
for l in phaselines:
  station = l[1:5].strip ()
  phasen  = l[9:15].strip ()
  hrmm    = l[18:22].replace (' ', '0').strip ()
  secs    = l[23:28].replace (' ', '0').strip ()
  print "=> Station: %s, phase: %s, time: %s:%s" % (station, phasen, hrmm, secs)
  phases.append ([station, phasen, hrmm, secs])


print "Reading positions and creating station file.."
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

positions = dict()
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

      #print "prev: " + prev
      #print "next: " + next

      prevf = prev.split (',')
      nextf = next.split (',')

      # do a linear interpolation between points
      latitude = []
      latitude.append (ddmm_mm_decimaldegree (prevf[4], prevf[5]))
      latitude.append (ddmm_mm_decimaldegree (nextf[4], nextf[5]))

      longitude = []
      longitude.append (ddmm_mm_decimaldegree (prevf[6], prevf[7]))
      longitude.append (ddmm_mm_decimaldegree (nextf[6], nextf[7]))

      t = []
      t.append (float(prevf[2]))
      t.append (float(nextf[2]))
      tdiff = t[1] - t[0]

      latitude_final = latitude[0] + ((latitude[1] - latitude[0]) / tdiff) * (hts - t[0])
      longitude_final = longitude[0] + ((longitude[1] - longitude[0]) / tdiff) * (hts - t[0])
      print "=> %s, interpolated latitude: %f and longitude: %f" % (s, latitude_final, longitude_final)

      positions[s] = [latitude_final, longitude_final]

      break

  if not found:
    print "=> No IP phase for station: %s, exiting." % s
    sys.exit (1)

# Seismometer is placed at station 2
positions[seismo] = positions['GAK2']
stations.append (seismo)

copy (os.path.join (datadir, '%s_STATION0.HYP' % jobid), os.path.join (jobd, 'STATION0.HYP'))
print "=> Writing station file (based on template): STATION0.HYP.."
stationsf = open (os.path.join (jobd, 'STATION0.HYP'), 'a')
for s in stations:
  # convert to DDMMMMM (where last three M is decimals without decimal point of
  # minute)
  latitude  = int(decimaldegree_ddmm_mmm(positions[s][0]))
  longitude = int(decimaldegree_ddmm_mmm(positions[s][1]))

  stationsf.write ('  {station:<4s}{latitude:>7d}{northsouth:s}{longitude: >8d}{eastwest:s}{height:4d}\n'.format (
      station   = s,
      latitude  = latitude,
      northsouth = ('N' if (positions[s][0] > 0) else 'S'),
      longitude = longitude,
      eastwest  = ('E' if (positions[s][1] > 0) else 'W'),
      height = 0
    ))

# write velocity model
vel_append = open (os.path.join (datadir, '%s_append' % jobid), 'r')
vel = vel_append.readlines ()
vel_append.close ()
for l in vel:
  stationsf.write (l)


stationsf.close ()

# For plotting
print "=> Writing station file: stations.dat.."
stationsf = open (os.path.join (jobd, 'stations.dat'), 'w')
for s in stations:
  # convert to DDMMSS.SS
  latitude  = decimaldegree_ddmmss(positions[s][0])
  longitude = decimaldegree_ddmmss(positions[s][1])

  stationsf.write ('{station:<5s} {latitude:8.1f}{northsouth:s} {longitude:8.1f}{eastwest:s} {height:6.1f}\n'.format (
      station   = s,
      latitude  = float(latitude),
      northsouth = ('N' if (positions[s][0] > 0) else 'S'),
      longitude = float(longitude),
      eastwest  = ('E' if (positions[s][1] > 0) else 'W'),
      height = 0.0
    ))

stationsf.close ()

# Generate job script
#jfuri = os.path.join (jobd, 'hyposatjob.sh')
#print "Writing HYPOSAT job file: hyposatjob.sh.."
#jf = open (jfuri, 'w')
#jf.write ('#! /bin/bash\n')
#jf.write ('# Automatically generated file (makehypojob.py): %s\n' % datetime.datetime.strftime (datetime.datetime.now(), '%Y-%m-%d %H:%M'))
#jf.write ('export HYPOSAT_DATA=%s\n' % os.path.join (datadir, 'data'))
#jf.write ('hyposat hyposat.in\n')
#jf.close ()
#os.chmod (jfuri, 0755)




