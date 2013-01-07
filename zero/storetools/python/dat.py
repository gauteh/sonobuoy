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

  def read (self, datf):
    # datf is file name
    indf = datf[:-3] + '.IND'

    print "dat: loading %s.." % datf,

    f = open (datf, 'rb')


    f.close ()

    print "loaded %d batches." % len(self.batches)





