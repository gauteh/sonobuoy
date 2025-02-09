#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-12-23
#
# bdata.py: Buoy data file
#

# constants
sample_length     = 4
batch_length      = 1024
batches_per_file  = 40

# structures extended with classes
class Bdata:
  class Batch:
    length  = batch_length
    no      = 0
    ref     = 0 # timestamp
    status  = 0
    latitude  = None
    longitude = None
    checksum  = 0

    hasclipped = False
    checksum_pass = False
    dataquality = None

    fixedtime = False
    notimefix = False
    origtime  = None

    samples_u = None
    samples_i = None

    def __init__ (self):
      self.samples_u = []
      self.samples_i = []

  # batches in bdata
  batches = None

  source  = 0 # 0 = DTT, 1 = DAT
  fixedtime = False
  notimefix = False
  checksum_pass = False

  localversion  = None
  remoteversion = None
  id            = None
  batchcount    = None
  hasfull       = None
  e_sdlag       = None
  batchlength   = 1024
  nobatches     = 0

  totalsamples  = 0

  def __init__ (self):
    self.batches = []

  def checksums (self):
    self.checksum_pass = True

    for b in self.batches:
      c = 0
      for u in b.samples_u:
        c ^= u

      if c == b.checksum:
        b.checksum_pass     = True
        self.checksum_pass &= True
      else:
        print "Error: Failed checksum on batch: %d" % b.no

  def int32 (self, x):
    if x>0xFFFFFFFF:
      raise OverflowError
    if x>0x7FFFFFFF:
      x=int(0x100000000-x)
      if x<2147483648:
        return -x
      else:
        return -2147483648
    return x

  def populate_int32_samples (self):
    for b in self.batches:
      for u in b.samples_u:
        b.samples_i.append (self.int32 (u))
