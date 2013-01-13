#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# Use OBSPY to create miniseed files from DATs or MATLAB files
#
# Usage:
#
#   makemseed.py [-p][-n] [-r root] station range [destination]
#
#     -p              plot trace
#     -n              do not write stream
#     station         station trace is for
#     -r root         is root directory of DAT files
#     range           is a range of ids of DAT files expected
#     destination     sub directory to put output (will be created)
#
#
#   NOT IMPLEMENTED:
#   makemseed.py [-p][-n] station file time-variable dataseries-variable
#                [destination]
#
#     -p              plot trace
#     -n              do not write stream
#     station         station trace is for
#     file            is matlab .mat file
#     time-variable   variable with time series
#     data-variable   variable with data series
#

import os
import sys

from obspy            import read, Stream, Trace, UTCDateTime
from obspy.core.trace import Stats

import numpy

from dat import *

class Makemseed:
  station = None
  network   = 'GB'
  location  = 'G3'
  sampling_rate = 250.0
  channel   = 'BNR'

  optplot     = False
  optnowrite  = False
  root        = "./"
  destdir     = "./"

  ids     = None
  name    = None
  start   = None
  bdatas  = None
  st      = None

  def __init__ (self):
    pass

  def go (self):
    # parse args
    if '-p' in sys.argv:
      self.optplot = True
      sys.argv.remove ('-p')

    if '-n' in sys.argv:
      self.optnowrite = True
      sys.argv.remove ('-n')
      print "mkms: (not writing stream)"

    if '-r' in sys.argv:
      self.root = sys.argv[sys.argv.index('-r') + 1]
      sys.argv.remove ('-r')
      sys.argv.remove (self.root)

    if len (sys.argv) < 3:
      print "Incorrect arguments."
      sys.exit (1)

    self.station = sys.argv[1]

    if '.mat' in sys.argv[2]:
      self.domat ()

    else:
      self.parserange ()
      self.dorange ()

  def plot (self):
    if self.st is not None:
      print "mkms: plotting.."
      self.st.plot (number_of_ticks = 300)

  def domat (self):
    print "Error: Not implemented."
    pass

  def parserange (self):
    # parse range
    if len(sys.argv) == 3:
      rrange = sys.argv[2]

    elif len(sys.argv) == 4:
      self.destdir   = sys.argv[3]
      rrange = sys.argv[2]

    self.ids = []

    for i in rrange.split (','):
      if '-' in i:
        aa  =  i.split ('-')
        self.ids += range (int(aa[0]), int(aa[1]) + 1)
      else:
        self.ids.append (int (i))

  def dorange (self):
    # load batches
    print "mkms: loading batches.."
    self.bdatas = []

    for i in self.ids:
      d = Dat ()
      d.read (os.path.join (self.root, str(i) + '.DAT'))

      self.bdatas.append (d.bdata)

    # set up datastream
    print "mkms: setting up stream for %s.." % self.station,
    self.st = Stream ()
    for bd in self.bdatas:
      for b in bd.batches:
        s = Stats ()
        s.sampling_rate = self.sampling_rate
        s.npts = b.length
        s.network = self.network
        s.location = self.location
        s.station = self.station
        s.channel = self.channel
        s.starttime = UTCDateTime ((b.ref / 1000000.0))

        t = Trace (data = numpy.array (b.samples_i, dtype = numpy.int32), header = s)
        self.st.append (t)

    print "done."

    # generate file name
    self.name = self.st[0].id.replace ('.', '_')

    self.start = self.st[0].stats.starttime
    self.name = self.start.strftime ("%Y-%m-%d-%H%M-%S") + '.' + self.name

    if self.optplot:
      self.plot ()

    if not self.optnowrite:
      print "mkms: writing %s.mseed.." % self.name,

      if not os.path.exists (self.destdir):
        os.makedirs (self.destdir)

      self.st.write (os.path.join (self.destdir, self.name + '.mseed'), format = 'MSEED', encoding = 'INT32', byteorder = 1, flush = 1, verbose = 0)

      print "done."

      # write ids and refs
      idsf = open (os.path.join (self.destdir, self.name + '.ids'), 'w')
      refsf = open (os.path.join (self.destdir, self.name + '.refs'), 'w')
      for bd in self.bdatas:
        idsf.write ("%d,%d\n" % (bd.id, 1 if bd.e_sdlag else 0))

        for b in bd.batches:
          refsf.write ("%d,%d,%d,%d,%s,%s,%s,%s,%s,%d\n" % (bd.id, b.no, b.ref, b.status, b.latitude[:-2], b.latitude[-2:], b.longitude[:-2], b.longitude[-2:], b.checksum, 1 if b.checksum_pass else 0))

      idsf.close ()
      refsf.close ()

      return (self.name + '.mseed', idsf, refsf)
    else:
      print "mkms: would write %s.mseed (disabled)." % os.path.join (self.destdir, self.name)
      return None

if __name__ == '__main__':
  m = Makemseed ()
  m.go ()

