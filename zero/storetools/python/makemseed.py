#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# Use OBSPY to create miniseed files from DATs or MATLAB files
#
# Usage:
#
#   makemseed.py root range
#
#     root  is root directory of DAT files
#     range is a range of ids of DAT files expected
#
#
#   makemseed.py file time-variable dataseries-variable
#
#     file            is matlab .mat file
#     time-variable   variable with time series
#     data-variable   variable with data series
#

import os
import sys

from dat import *

class Makemseed:
  def __init__ (self):
    pass

  def go (self):
    # parse args
    if len (sys.argv) < 2:
      print "Incorrect arguments."
      sys.exit (1)

    if '.mat' in sys.argv[1]:
      self.domat ()

    else:
      self.dorange ()

  def domat (self):
    print "Error: Not implemented."
    pass

  def dorange (self):
    # parse range
    if len(sys.argv) == 2:
      root   = './'
      rrange = sys.argv[1]

    elif len(sys.argv) == 3:
      root   = sys.argv[1]
      rrange = sys.argv[2]

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

if __name__ == '__main__':
  m = Makemseed ()
  m.go ()

