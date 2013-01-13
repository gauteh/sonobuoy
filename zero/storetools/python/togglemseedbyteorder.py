#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-01-13
#
# Use OBSPY to toggle byte order on mseeds (because of faulty export
# to be compatible by buggy SEISAN 9.0..)
#
# Usage:
#
#   togglemseedbyteorder.py [-r] [-p] [-n] mseedfile.mseed
#
#     -r      replace original file (otherwise create a new file)
#     -p      plot original and new
#     -n      do not write out new yet
#

import os
import sys

from obspy            import read, Stream, Trace, UTCDateTime
from obspy.core.trace import Stats

import numpy

class ToggleMseedByteOrder:
  mseedf  = None
  replace = False
  nowrite = False
  plot    = False
  st      = None
  newst   = None

  def __init__ (self):
    pass

  def byteswap (self):
    pass

  def setupplot (self):
    if self.st is not None:
      print "tmbo: plotting.."
      self.st.plot (number_of_ticks = 300)


  def run (self):
    # load mseed file
    print "tmbo: loading mseed: %s.." % self.mseedf
    self.st = read (self.mseedf)

    if self.plot:
      self.setupplot ()

    # byte swap
    self.newst = Stream ()


if __name__ == '__main__':
  t = ToggleMseedByteOrder ()

  if '-r' in sys.argv:
    t.replace = True
    sys.argv.remove ('-r')

  if '-p' in sys.argv:
    t.plot = True
    sys.argv.remove ('-p')

  if '-n' in sys.argv:
    t.nowrite = True
    sys.argv.remove ('-n')

  if len(sys.argv) != 2:
    print "Incorrect arguments."

  t.mseedf = sys.argv[1]
  t.run ()


