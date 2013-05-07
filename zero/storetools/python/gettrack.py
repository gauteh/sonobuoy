#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-05-07
#
# Create track CSV file from ids 
#
# Usage:
#
#   gettrack.py [-r root] station range [destination]
#
#     station         station trace is for
#     -r root         is root directory of DAT files
#     range           is a range of ids of DAT files expected
#     destination     sub directory to put output (will be created)
#
#

import os
import sys

from obspy            import read, Stream, Trace, UTCDateTime
from obspy.core.trace import Stats

import numpy

from dat import *

class GetTrack:
  station = None
  sampling_rate = 250.0

  root        = "./"
  destdir     = "./"

  ids     = None
  name    = None
  start   = None
  bdatas  = None
  st      = None

  outfile = ""

  def __init__ (self):
    pass

  def go (self):
    # parse args
    if '-r' in sys.argv:
      self.root = sys.argv[sys.argv.index('-r') + 1]
      sys.argv.remove ('-r')
      sys.argv.remove (self.root)

    if len (sys.argv) < 3:
      print "Incorrect arguments."
      sys.exit (1)

    self.station = sys.argv[1]
    self.outfile = '%(station)s_track.csv' % { 'station' : self.station }
    self.outfile_t = '%(station)s_track.tab' % { 'station' : self.station }

    print "gt: writing to file: " + self.outfile
    print "gt: writing to file: " + self.outfile_t

    self.parserange ()
    self.dorange ()

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
    print "gt: opening output file.."
    self.outfile = os.path.join (self.destdir, self.outfile)
    self.outfile_t = os.path.join (self.destdir, self.outfile_t)
    f = open (self.outfile, 'w')
    ft = open (self.outfile_t, 'w')

    print "gt: loading batches.."
    print "gt: writing to file: " + self.outfile
    print "gt: writing to file: " + self.outfile_t

    for i in self.ids:
      d = Dat ()
      d.read (os.path.join (self.root, str(i) + '.DAT'))

      bd = d.bdata
      for b in bd.batches:
        f.write ("%d,%d,%d,%d,%s,%s,%s,%s,%s,%d\n" % (bd.id, b.no, b.ref, b.status, b.latitude[:-2], b.latitude[-2:], b.longitude[:-2], b.longitude[-2:], b.checksum, 1 if b.checksum_pass else 0))
        ft.write ('%s%s     %s%s\n' % ( b.longitude[:-2], b.longitude[-2:], b.latitude[:-2], b.latitude[-2:]))

    f.close ()
    ft.close ()

if __name__ == '__main__':
  m = GetTrack ()
  m.go ()

