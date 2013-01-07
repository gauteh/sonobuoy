#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# Use OBSPY to create miniseed files from DATs or MATLAB files
#
# Usage:
#
#   makemseed.py station root range
#
#     station         station trace is for
#     root  is root directory of DAT files
#     range is a range of ids of DAT files expected
#
#
#   makemseed.py station file time-variable dataseries-variable
#
#     station         station trace is for
#     file            is matlab .mat file
#     time-variable   variable with time series
#     data-variable   variable with data series
#

import os
import sys

from obspy import read, Stream, Trace, UTCDateTime
from obspy.core.trace import Stats

import numpy

from dat import *

class Makemseed:
  station = None
  network   = 'GB'
  location  = 'G3'
  sampling_rate = 250.0
  channel   = 'BNR'

  def __init__ (self):
    pass

  def go (self):
    # parse args
    if len (sys.argv) < 3:
      print "Incorrect arguments."
      sys.exit (1)

    self.station = sys.argv[1]

    if '.mat' in sys.argv[2]:
      self.domat ()

    else:
      self.dorange ()

  def domat (self):
    print "Error: Not implemented."
    pass

  def dorange (self):
    # parse range
    if len(sys.argv) == 3:
      root   = './'
      rrange = sys.argv[2]

    elif len(sys.argv) == 4:
      root   = sys.argv[2]
      rrange = sys.argv[3]

    ids = []

    for i in rrange.split (','):
      if '-' in i:
        aa  =  i.split ('-')
        ids += range (int(aa[0]), int(aa[1]) + 1)
      else:
        ids.append (int (i))

    # load batches
    bdatas = []

    for i in ids:
      d = Dat ()
      d.read (os.path.join (root, str(i) + '.DAT'))

      bdatas.append (d.bdata)

    # set up datastream
    print "mkms: setting up stream for %s.." % self.station,
    st = Stream ()
    for bd in bdatas:
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
        st.append (t)

    print "done."

    # generate file name
    name = st[0].id.replace ('.', '_')

    start = st[0].stats.starttime
    name = start.strftime ("%Y-%m-%d-%H%M-%S") + '.' + name

    print "Writing %s.mseed.." % name,

    st.write (name + '.mseed', format = 'MSEED', encoding = 'INT32', byteorder = 1, flush = 1, verbose = 0)

    print "done."

    # write ids and refs
    idsf = open (name + '.ids', 'w')
    refsf = open (name + '.refs', 'w')
    for bd in bdatas:
      idsf.write ("%d,%d\n" % (bd.id, 1 if bd.e_sdlag else 0))

      for b in bd.batches:
        refsf.write ("%d,%d,%d,%d,%s,%s,%s,%s,%s,%d\n" % (bd.id, b.no, b.ref, b.status, b.latitude[:-2], b.latitude[-2:], b.longitude[:-2], b.longitude[-2:], b.checksum, 1 if b.checksum_pass else 0))

    idsf.close ()
    refsf.close ()

if __name__ == '__main__':
  m = Makemseed ()
  m.go ()

