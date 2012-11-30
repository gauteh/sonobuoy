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

# matplotlib and basemap
from mpl_toolkits.basemap import Basemap
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.text as text

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
#m = Basemap (projection = 'npstere', boundinglat = 80, lon_0 = 0, resolution = 'l')
#m = Basemap (projection = 'lcc', resolution = 'l',
             #llcrnrlon  = -80,
             #llcrnrlat  = 0,
             #urcrnrlon   = 80,
             #urcrnrlat   = 90,
             #lat_0 = 84.,
             #lon_0 = 4.
            #)
m = Basemap (width=2200000,height=1200000,
             rsphere=(6378137.00,6356752.3142),\
             resolution='l',area_thresh=1000.,projection='lcc',\
             lat_1=-10.,lat_2=80,lat_0=84,lon_0=4)

#m.drawcoastlines ()
#m.fillcontinents (color = 'coral', lake_color = 'aqua')

m.drawparallels (np.arange (80., 90., 2.0), labels = [False, True, True, False], color = 'white')
m.drawmeridians (np.arange (-180.0, 180., 10), latmax = 90, labels = [True, False, False, True], color = 'white')

plt.title ('Hypocenter solutions for event: %s' % event)

plt.hold (True)

# stationcolors
stationcolors = { 'GAK2' : 'w', 'GAK3' : 'g', 'GAK4' : 'r' }

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

      #m.tissot (lon, lat, 0.01, 100, facecolor = 'green', zorder = 10, alpha = 1)

      x, y = m(lon, lat)
      m.plot (x, y, 'o' + stationcolors[name], label = name, markersize = 10, alpha = 0.7)

      if 'GAK2' in name:
        name = 'GAK2 (GAKS)'

      plt.text (x+250, y, name, rotation = -15, color = 'white')

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

  print "t0: %(t0)s, lat: %(lat)g, lon: %(lon)g, depth: %(depth)g, vpvs: %(vpvs)g, rms: %(rms)g" % { 't0' : t0, 'lat' : lat, 'lon' : lon, 'depth' : z, 'vpvs' : vpvs, 'rms' : rms}

  # plot origin
  x, y = m(lon, lat)
  m.plot (x, y, '*y', label = event + '_' + j, markersize = 15)


m.bluemarble ()
plt.legend ()
plt.show ()


