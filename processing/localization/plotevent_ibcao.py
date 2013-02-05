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

# stationcolors (defined in stations.cpt in gmt folder)
stationcolorscode = { 'GAK2' : '0', 'GAK3' : '1', 'GAK4' : '2' }
stationcolors = { 'GAK2' : 'green', 'GAK3' : 'yellow', 'GAK4' : 'black' }

# quakes
psf = open (os.path.join (mapdir, 'stations.d'), 'w')
pstf = open (os.path.join (mapdir, 'stations.t'), 'w')
pqf = open (os.path.join (mapdir, 'quakes.d'), 'w')
pqe = open (os.path.join (mapdir, 'quakes.e.d'), 'w')

legf = open (os.path.join (mapdir, 'legend.txt'), 'w')

legf.write ("H 20 - Event\n")
legf.write ("L 10 - C %s\n" % event)
legf.write ("D 0.1c 0.1p\n")

jobcolorcodes = [0, 1, 2, 3]
jobcolors     = ['red', 'blue', 'cyan', 'black']

# Plotting jobs
jobno = 0
stationsplotted = False
jobs.sort ()

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

      if 'GAK2' in name:
        rname = 'GAK2 (GAKS)'
      else:
        rname = name

      # plot station
      psf.write ("%4.2f %4.2f %s\n" % (lon, lat, stationcolorscode[name]))
      pstf.write ("%4.2f %4.2f 4 0 20 BL %s\n" % (lon, lat, name))

      # write to legend
      legf.write ("S 5p t 7p %s 0.1p 0.5c %s\n" % (stationcolors[name], rname))


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

  pqf.write ("%4.2f %4.2f %d\n" % (lon, lat, jobno))
  pqe.write ("%4.2f %4.2f %d 129.5 0.25 0.23 \n" % (lon, lat, jobno))
  # write to legend
  legf.write ("D 0.1c 0.1p\n")
  legf.write ("S 5p a 7p %s 0.1p 0.5c Epicenter (rms: %g, job: %s) \n" % (jobcolors[jobno], rms, j))
  legf.write ("L 8 8 L Epicenter: %gN, %gE\n" % (lat, lon))
  legf.write ("L 8 8 L Depth:     %g [km] (fixed)\n" % z)
  legf.write ("L 8 8 L Origin:    %s\n" % t0)
  #legf.write ("L 8 8 L RMS:       %g (job: %s)\n" % (rms, j))

  print "t0: %(t0)s, lat: %(lat)g, lon: %(lon)g, depth: %(depth)g, vpvs: %(vpvs)g, rms: %(rms)g" % { 't0' : t0, 'lat' : lat, 'lon' : lon, 'depth' : z, 'vpvs' : vpvs, 'rms' : rms}

  jobno += 1

legf.write ("D 0.1c 0.1p\n")
legf.write ("L 8 - C EXPERIMENTAL solution using HYPOSAT.\n");

psf.close ()
pstf.close ()
pqf.close ()
pqe.close ()
legf.close ()

bigi = os.path.join (datadir, 'plotgmt_big.sh')
regi = os.path.join (datadir, 'plotgmt_reg.sh')
deti = os.path.join (datadir, 'plotgmt_detail.sh')

pr = subprocess.Popen ([deti], cwd = mapdir)
pr.wait ()

pr = subprocess.Popen ([regi], cwd = mapdir)
pr.wait ()

#pb = subprocess.Popen ([bigi], cwd = mapdir)
#pb.wait ()



