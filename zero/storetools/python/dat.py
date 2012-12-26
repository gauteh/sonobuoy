#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# dat.py: Interface to DAT files
#

import os

class Dat:
  batches = []

  def __init__ (self):
    pass

  def read (self, s):
    # s is file name
    print "dat: loading %s.." % s

    f = open (s, 'r')




