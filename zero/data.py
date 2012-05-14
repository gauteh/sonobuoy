# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-05-14
#
# data.py: Keep track of one data file for buoy

import threading
import logging
import time
import os

class Data:
  buoy  = None
  index = None
  logger = None

  id = 0
  enabled = False

  class Ref: # Ref struct {{{
    id  = 0
    no  = 0
    ref      = None
    status   = None
    complete = False
    lineno   = 0 # line no in ref

    def __init__ (self, _i, _n, _r, _s, _c):
      self.id = _i
      self.no = _n
      self.ref = _r
      self.status = _s
      self.complete = _c

    def __eq__ (self, other):
      return (self.no == other) # compare against ref no
  # }}}

  refs = [] # Known refs available on buoy, with flag indicating complete
            # download.

  indexf_uri  = None
  dataf_uri   = None
  indexf      = None # Index file for this data segment (refs + meta)
  dataf       = None # Samples with refs

  store_version = 0
  index_id      = 0
  sample_length = 0
  samples       = 0
  batch_length  = 0
  refs_no       = 0

  def __init__ (self, l, _buoy, _index, _id, _enabled):
    self.logger = l
    self.buoy = _buoy
    self.index = _index
    self.id = _id
    self.enabled = _enabled


    self.indexf_uri = os.path.join (self.buoy.logdir, str(self.id) + '.ITT')
    self.dataf_uri  = os.path.join (self.buoy.logdir, str(self.id) + '.DTT')

    self.logger.info ("[Data] [" + str(self.id) + "] Initializing, reading index and data..")
    self.read_index ()
    self.read_data  ()

  ''' Read index file and figure out meta data and existing refs {{{

      Format (as returned by GETID):
      Store version
      Index id
      Sample length
      Samples
      Batch length
      No of refs

      After this, one line for each ref (as returned by GETREFS):
      id (always the same),ref no, ref
  '''
  def read_index (self):
    if os.path.exists (self.indexf_uri):
      self.indexf = open (self.indexf_uri, 'r')

      self.store_version  = int(self.indexf.readline ())
      self.index_id       = int(self.indexf.readline ())
      self.sample_length  = int(self.indexf.readline ())
      self.samples        = int(self.indexf.readline ())
      self.batch_length   = int(self.indexf.readline ())
      self.refs_no        = int(self.indexf.readline ())

      for l in self.indexf.readlines ():
        s = l.split (',')
        self.refs.append (Ref (int(s[0]), int(s[1]), int(s[2]), 0, False))

      self.indexf.close ()
      self.refs = sorted (self.refs, key = lambda r: r.no)

  def write_index (self):
    self.refs = sorted (self.refs, key = lambda r: r.no)
    self.indexf = open (self.indexf_uri, 'w+') # truncate file
    self.indexf.write (str(self.store_version) + '\n')
    self.indexf.write (str(self.index_id) + '\n')
    self.indexf.write (str(self.sample_length) + '\n')
    self.indexf.write (str(self.samples) + '\n')
    self.indexf.write (str(self.batch_length) + '\n')
    self.indexf.write (str(self.refs_no) + '\n')
    for i in self.refs:
      self.indexf.write (str(i.id) + ',' + str(i.no) + ',' + str(i.ref) + '\n')

    self.indexf.close ()
  # }}}

  ''' Read data file and figure out which references are complete '''
  def read_data (self):
    if os.path.exists (self.dataf_uri):
      self.dataf = open (self.dataf_uri, 'r')

      # Scan for references, all that are present here must be complete
      n = 0
      for l in self.dataf.readlines ():
        if l[0] == 'R':
          s     = l.split (',')
          bl    = s[1]
          refno = s[2]
          ref   = s[3]
          stat  = s[4]

          # Find the ref in refslist
          if ref in self.refs:
            i = self.refs.index(refno)
            self.refs[i].complete = True
            self.refs[i].lineno   = n

        n = n + 1 # line no

  def append_batch (self, batch_length, reference, status, samples):
    # reference is time
    # samples is list of samples for this ref

    # find Ref corresponding to reference
    ref = None
    for i in self.refs:
      if i.ref == reference:
        ref = i

    if ref is None:
      self.logger.error ("[Data] Got batch without corresponding reference.")
      return

    # load data file
    # write segment by segment
    # insert new batch
    # update linenos and refs
    lines = []
    if os.path.exists (self.dataf_uri):
      self.dataf = open (self.dataf_uri, 'r')

      lines = self.dataf.readlines ()
      self.dataf.close ()

    self.dataf = open (self.dataf_uri, 'w+') # truncate

    newref_written = False
    n = 0 # lineno
    for i in self.refs:
      # Write new batch
      if ref.no < i.no:
        # Write ref
        self.dataf.writeline ("R," + str(self.batch_length) + "," + str(ref.no) + "," + str(ref.ref) + "," + str(ref.status))
        ref.lineno    = n + 1
        ref.complete  = True
        # Write samples
        for s in samples:
          self.dataf.writeline (str(samples))

        newref_written = True
        n = n + self.batch_length + 1

      else:
        if i.complete:
          k = self.batch_length + 1
          j = 0
          while j < k:
            self.dataf.writeline (lines[i.lineno + j])
            j = j + 1
            n = n + 1

          # Update lineno if new batch has been written
          if newref_written:
            i.lineno = i.lineno + self.batch_length + 1

    self.dataf.close ()

    self.refs.append (ref)
    self.write_index ()

  def __eq__ (self, other):
    return (self.id == other)

  def close (self):
    self.write_index ()

