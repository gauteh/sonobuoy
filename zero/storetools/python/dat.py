#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# dat.py: Interface to DAT files
#

import os
from struct import *
from array import array

from bdata import *

class Dat:
  bdata = None

  def __init__ (self):
    self.bdata = Bdata ()

  def read (self, datfu):
    # datf is file name
    indfu = datfu[:-3] + 'IND'

    print "dat: loading %s.." % datfu,

    # read index
    indf = open (indfu, 'rb')

    self.bdata.source = 1
    self.bdata.localversion = 0

    self.bdata.remoteversion, = unpack ('H', indf.read (2))
    self.bdata.id,            = unpack ('I', indf.read (4))

    indf.read (2) # skip sample length
    self.bdata.totalsamples,  = unpack ('I', indf.read (4))
    self.bdata.batchlength,   = unpack ('I', indf.read (4))
    self.bdata.nobatches,     = unpack ('I', indf.read (4))
    self.bdata.e_sdlag,       = unpack ('?', indf.read (1))

    indf.close ()

    datf = open (datfu, 'rb')
    nref = 0
    while nref < self.bdata.nobatches:
      b = Bdata.Batch ()

      # read reference  (repeat)
      datf.read (3 * sample_length) # skip padding

      b.no,     = unpack ('I', datf.read (4))
      b.ref,    = unpack ('Q', datf.read (8))
      b.status, = unpack ('I', datf.read (4))
      b.latitude = datf.read (11)
      datf.read (1) # skip extra lat byte
      b.longitude = datf.read (12)
      b.checksum, = unpack ('I', datf.read (4))

      datf.read (3 * sample_length) # skip padding

      if b.no != nref:
        print "Error: Reference number does not match reference number in file."
        print b.no
        print nref
        break

      # read samples    (repeat)
      b.samples_u = array ('I')
      b.samples_u.read (datf, b.length)

      self.bdata.batches.append (b)
      nref += 1

    datf.close ()

    self.bdata.populate_int32_samples ()
    self.bdata.checksums ()

    print "loaded %d batches (%d samples, version: %d, sdlag: %s)." % (self.bdata.nobatches, self.bdata.totalsamples, self.bdata.remoteversion, ("yes" if self.bdata.e_sdlag else "no"))

