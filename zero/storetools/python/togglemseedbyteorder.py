#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-01-13
#
# Use OBSPY to toggle byte order on mseeds (because of faulty export
# to be compatible by buggy SEISAN 9.0..)
#
# Usage:
#
#   togglemseedbyteorder.py [-r] [-p] [-n] [-b] mseedfile.mseed
#
#     -r      replace original file (otherwise create a new file)
#     -p      plot original and new
#     -n      do not write out new or back up yet
#     -b      back-up original mseed (append .bs.bak)
#

import os
import sys
import shutil

from obspy            import read, Stream, Trace, UTCDateTime
from obspy.core.trace import Stats

import numpy

class ToggleMseedByteOrder:
  mseedf  = None
  backup  = False
  replace = False
  nowrite = False
  plot    = False
  st      = None
  newst   = None
  name    = ""

  def __init__ (self):
    pass

  def byteswap (self):
    print "tmbo: byte swapping each trace.."
    self.newst = self.st.copy ()

    # work through all traces and byte swap
    for t in self.newst:
      print "tmbo: swapping %s (dtype: %s)" % (t, t.data.dtype)

      if t.data.dtype != numpy.dtype ('int32'):
        print "tmbo: wrong dtype: %s (should be int32)." % t.data.dtype
        sys.exit (1)

      t.data = t.data.byteswap ()

  def setupplot (self):
    print "tmbo: plotting.."
    if self.st is not None:
      self.st.plot (block = False)

    if self.newst is not None:
      self.newst.plot ()

  def run (self):
    # load mseed file
    print "tmbo: loading mseed: %s.." % self.mseedf
    self.st = read (self.mseedf)

    # byte swap
    self.byteswap ()

    if self.plot:
      self.setupplot ()

    # figure out new name
    root = os.path.dirname (self.mseedf)
    base = os.path.basename (self.mseedf)
    self.name = base

    if not self.replace:
      self.name += '.new'

    if not self.nowrite:
      if self.replace and self.backup:
        print "tmbo: backing up existing file to: %s.." % (self.mseedf + '.bs.bak')
        os.rename (self.mseedf, self.mseedf + '.bs.bak')


      # writing
      print "tmbo: writing new file: %s.." % os.path.join (root, self.name)
      self.newst.write (os.path.join (root, self.name), format = 'MSEED', encoding = 'INT32', byteorder = 1, flush = 1, verbose = 0)

    else:
      if self.replace and self.backup:
        print "tmbo: backing up existing file to: %s (disabled).." % (self.mseedf + '.bs.bak')

      print "tmbo: writing new file: %s (disabled).." % os.path.join (root, self.name)

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

  if '-b' in sys.argv:
    t.backup = True
    sys.argv.remove ('-b')

  if len(sys.argv) != 2:
    print "Incorrect arguments."

  t.mseedf = sys.argv[1]
  t.run ()


