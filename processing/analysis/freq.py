#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# Use OBSPY to plot trace and freq analyze 
#
# Usage:
#
#   freq.py [-p][-n] [-r root] mseed files
#
#     -p              plot trace
#     station         station trace is for
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

class Freq:
  streams = Stream()

  def __init__ (self):
    pass

  def go (self):
    # parse args
    if '-p' in sys.argv:
      self.optplot = True
      sys.argv.remove ('-p')

    if '-r' in sys.argv:
      self.root = sys.argv[sys.argv.index('-r') + 1]
      sys.argv.remove ('-r')
      sys.argv.remove (self.root)

    self.files = sys.argv[1:]
    print "freq: working on files: " + str(self.files)
    self.load ()


    self.plot ()


  def plot (self):
    self.streams.merge ()
    self.streams.filter ('highpass', freq=0.1)

    print "freq: plotting.."
    self.streams.plot (block = False)

    print "freq: spectrogram.."
    self.streams.spectrogram ()


  def load (self):
    for f in self.files:
      self.streams += read (f)




if __name__ == '__main__':
  m = Freq ()
  m.go ()

