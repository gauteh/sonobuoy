#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-27
#
# Plot one event using Matplotlib and Basemap
#
# Run from event dir or with event as first argument,
# all job_* with solutions will be plotted.
#

import os
import os.path
import sys
import shutil
import subprocess

# local
from utils import *

eventdir = './'
event    = os.path.basename (os.getcwd())

if len(sys.argv) > 1:
  eventdir  = os.path.join (eventdir, sys.argv[1])
  event     = sys.argv[1]

eventuri = os.path.join (eventdir, event)

print "Plotting event: %s.." % event

if not os.path.exists (eventuri):
  print "Could not open event: %s, exiting." % eventuri
  sys.exit (1)

plotdir = os.path.join (eventdir, 'plot')
if not os.path.exists (plotdir):
  os.makedirs (plotdir)

# Scan for jobs
print "Scanning for jobs.."
jobs = []
entries = os.listdir (eventdir)
for e in entries:
  if os.path.isdir (os.path.join (eventdir, e)):
    if 'job_' in e:
      jobs.append (e)

if len(jobs) < 1:
  print "--> no jobs, exiting."
  sys.exit (1)

# Setting up map
print "Setting up map.."
datadir = os.path.join (os.path.dirname (sys.argv[0]), 'gmt')
mapdir  = os.path.join (eventdir, 'map')
if not os.path.exists (mapdir):
  os.makedirs (mapdir)

# stationcolors
stationcolors = { 'GAK2' : 'w', 'GAK3' : 'g', 'GAK4' : 'r' }

# quakes
psf = open (os.path.join (mapdir, 'stations.d'), 'w')
pstf = open (os.path.join (mapdir, 'stations.t'), 'w')
pqf = open (os.path.join (mapdir, 'quakes.d'), 'w')


# Plotting jobs
stationsplotted = False
for j in jobs:
  jd = os.path.join (eventdir, j)

  # plot stations if not plotted already
  if not stationsplotted:
    print "Plotting stations.."
    sf = open (os.path.join (jd, 'stations.dat'), 'r')
    for l in sf.readlines ():
      ss = l.split (' ')
      name = ss[0]
      lat  = ss[2]
      lon  = ss[4]

      # is plotted along with GAK2
      if 'GAKS' in name:
        continue

      lat = ddmmss_decimaldegree (lat)
      lon = ddmmss_decimaldegree (lon)

      print "--> %s: %4.2f, %4.2f" % (name, lat, lon)

      #if 'GAK2' in name:
        #name = 'GAK2 (GAKS)'

      # plot station
      psf.write ("%4.2f %4.2f\n" % (lon, lat))
      pstf.write ("%4.2f %4.2f 4 -30 20 BL %s\n" % (lon, lat, name))


    stationsplotted = True
    print "Plotting jobs.."

  # Extract info from job
  print "--> %s:" % j,
  hypoout = os.path.join (jd, 'hyposat-out')
  if not os.path.exists (hypoout):
    print "No hyposat-out, skipping."
    continue

  hf = open (hypoout, 'r')
  lines = hf.readlines ()
  hf.close ()

  next = False
  for l in lines:
    if next:
      res = l

    elif l[:2] == 'T0':
      next = True

  if not next:
    print "No solution line found."
    continue

  # Parse result line
  t0    = res[0:23]
  lat   = float(res[26:32])
  lon   = float(res[36:41])
  z     = float(res[43:49])
  vpvs  = float(res[51:56])
  rms   = float(res[107:112])

  pqf.write ("%4.2f %4.2f\n" % (lon, lat))

  print "t0: %(t0)s, lat: %(lat)g, lon: %(lon)g, depth: %(depth)g, vpvs: %(vpvs)g, rms: %(rms)g" % { 't0' : t0, 'lat' : lat, 'lon' : lon, 'depth' : z, 'vpvs' : vpvs, 'rms' : rms}


psf.close ()
pstf.close ()
pqf.close ()

bigi = os.path.join (datadir, 'plotgmt_big.sh')
regi = os.path.join (datadir, 'plotgmt_reg.sh')

pr = subprocess.Popen ([regi], cwd = mapdir)
pr.wait ()

pb = subprocess.Popen ([bigi], cwd = mapdir)
pb.wait ()



