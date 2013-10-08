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
    if e[0:4] == 'job_':
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
stationcolors = { 'GAK2' : 'green', 'GAK3' : 'yellow', 'GAK4' : 'red' }

# quakes
psf = open (os.path.join (mapdir, 'stations.d'), 'w')
pstf = open (os.path.join (mapdir, 'stations.t'), 'w')
pqf = open (os.path.join (mapdir, 'quakes.d'), 'w')
pqe = open (os.path.join (mapdir, 'quakes.e.d'), 'w')

pqcf = open (os.path.join (mapdir, 'quakes.hs.d'), 'w')

legf = open (os.path.join (mapdir, 'legend.txt'), 'w')

legf.write ("H 20 - Event\n")
legf.write ("L 10 - C %s\n" % event)
legf.write ("D 0.1c 0.1p\n")

jobcolorcodes = [0, 1, 2, 3, 4, 5, 6, 7]
jobcolors     = ['red', 'blue', 'cyan', 'black', 'yellow', 'cyan', 'black', 'yellow']

# Plotting jobs
jobno = 0
stationsplotted = False
jobs.sort ()

# script for converting from cartesian to geographic coordinates
cart2geo = os.path.join (datadir, 'cart2geo.sh')

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
  hsout   = os.path.join (jd, 'hs_out')
  hcout   = os.path.join (jd, 'hyp.out')
  if os.path.exists (hypoout):
    hf = open (hypoout, 'r')
    lines = hf.readlines ()
    hf.close ()

    next = False
    for l in lines:
      azi = None

      if next:
        res = l
        # Parse result line
        t0    = res[0:23]
        lat   = float(res[26:32])
        lon   = float(res[36:41])
        z     = float(res[43:49])
        vpvs  = float(res[51:56])
        rms   = float(res[107:112])

      elif l[:2] == 'T0':
        next = True

      elif 'Major axes' in l:
        maj_ax = float(l[11:20].strip())
        min_ax = float(l[38:48].strip())

      elif 'Azimuth' in l:
        azi = float(l[10:20].strip())


    if not next:
      print "No solution line found."
      continue


    pqf.write ("%f %f %d\n" % (lon, lat, jobno))
    if (azi is not None):
      pqe.write ("%f %f %d %g %g %g\n" % (lon, lat, jobno, azi, maj_ax, min_ax))
    # write to legend
    legf.write ("D 0.1c 0.1p\n")
    legf.write ("S 5p a 7p %s 0.1p 0.5c Epicenter (rms: %g, %s) \n" % (jobcolors[jobno], rms, j))
    legf.write ("L 8 8 L Epicenter: %gN, %gE\n" % (lat, lon))
    legf.write ("L 8 8 L Depth:     %g [km] (fixed)\n" % z)
    legf.write ("L 8 8 L Origin:    %s\n" % t0)
    #legf.write ("L 8 8 L RMS:       %g (job: %s)\n" % (rms, j))

    print "t0: %(t0)s, lat: %(lat)g, lon: %(lon)g, depth: %(depth)g, vpvs: %(vpvs)g, rms: %(rms)g" % { 't0' : t0, 'lat' : lat, 'lon' : lon, 'depth' : z, 'vpvs' : vpvs, 'rms' : rms}

    jobno += 1

  elif os.path.exists (hsout):
    # convert to geographic coordinates
    pr = subprocess.Popen (["bash", cart2geo, hsout], stdout = subprocess.PIPE)

    lines = pr.stdout.readlines ()

    first = True

    for l in lines:
      if l[0] == '#':
        continue # comment

      c = l.find("#")
      l = l[:c] # strip comments

      # input is in UPS coordinates
      s = l.split (' ')
      for i in s:
        if i == '':
          s.remove (i)

      t0  = s[5] + ' ' + s[6]
      lon = float(s[0])
      lat = float(s[1])
      depth = float(s[2])
      rms = float(s[3])
      phasesused = int(s[4])

      phases = ''
      if phasesused == 0:
        phases = 'P,S'
      elif phasesused == 1:
        phases = 'P,S,M'
      elif phasesused == 2:
        phases = 'P,S,M,MM'

      if first:
        first = False
      else:
        print "     ",
        k = 0
        while k < len(j):
          print " ",
          k += 2

      print "t0: %(t0)s, lat: %(lat)g, lon: %(lon)g, depth: %(depth)g, rms: %(rms)g, phaseuse: %(phaseuse)d" % { 't0' : t0, 'lat' : lat, 'lon' : lon, 'depth' : depth,  'rms' : rms, 'phaseuse' : phasesused}

      pqcf.write ('%f %f %d\n' %(lon, lat, jobno))

      # write to legend
      legf.write ("D 0.1c 0.1p\n")
      legf.write ("S 5p a 7p %s 0.1p 0.5c Epicenter (rms: %4.3f, %s) \n" % (jobcolors[jobno], rms, j))
      legf.write ("L 8 8 L Epicenter: %gN, %gE\n" % (lat, lon))
      legf.write ("L 8 8 L Depth: %g [km], phases: %s\n" % ((depth / 1000), phases))
      legf.write ("L 8 8 L Origin: %s\n" % t0)

      jobno += 1


  elif os.path.exists (hcout):
    f = open (hcout, 'r')
    lines = f.readlines ()
    f.close ()

    l = lines[0]
    #l = l.split (' ')
    t0 = l[1:5] + '-' + str(int(l[6:8])) + '-' + str(int(l[8:10])) + ' ' + l[11:13] + ':' + l[13:15] + ':' + l[16:20]
    lat = float(l[24:30])
    lon = float(l[31:38])
    #lat = ddmm_mm_decimaldegree (lat[:-1], lat[-1])
    #lon = ddmm_mm_decimaldegree (lon[:-1], lon[-1])
    depth = float(l[39:43])
    rms   = float(l[52:55])


    print "t0: %(t0)s, lat: %(lat)g, lon: %(lon)g, depth: %(depth)g, rms: %(rms)g" % { 't0' : t0, 'lat' : lat, 'lon' : lon, 'depth' : depth,  'rms' : rms }

    pqcf.write ('%f %f %d\n' %(lon, lat, jobno))

    # write to legend
    legf.write ("D 0.1c 0.1p\n")
    legf.write ("S 5p a 7p %s 0.1p 0.5c Epicenter (rms: %4.3f, %s) \n" % (jobcolors[jobno], rms, j))
    legf.write ("L 8 8 L Epicenter: %gN, %gE\n" % (lat, lon))
    legf.write ("L 8 8 L Depth: %g [km]\n" % (depth))
    legf.write ("L 8 8 L Origin: %s\n" % t0)


    jobno += 1

  else:
    print "No hyposat-out or hyposearch solution, skipping."
    continue

#legf.write ("D 0.1c 0.1p\n")
#legf.write ("L 8 - C EXPERIMENTAL solution using HYPOSAT.\n");

psf.close ()
pstf.close ()
pqf.close ()
pqe.close ()
legf.close ()
pqcf.close ()


bigi = os.path.join (datadir, 'plotgmt_big.sh')
regi = os.path.join (datadir, 'plotgmt_reg.sh')
deti = os.path.join (datadir, 'plotgmt_detail.sh')

pr = subprocess.Popen ([deti], cwd = mapdir)
pr.wait ()

pr = subprocess.Popen ([regi], cwd = mapdir)
pr.wait ()

#pb = subprocess.Popen ([bigi], cwd = mapdir)
#pb.wait ()



