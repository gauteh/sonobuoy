# -*- coding: utf-8 -*-
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-05-14
#
# data.py: Keep track of one data file for buoy

import threading
import logging
import time
import os

BATCH_LENGTH = 1024
CHUNK_SIZE   = 512

class Batch:
  no      = 0   # no of ref/batch in data file
  ref     = None
  status  = None
  complete  = False # Is batch completely received
  line      = None   # Where in data file does this ref start


  completechunks  = [] # list of chunks received
  maxchunks       = (BATCH_LENGTH / CHUNK_SIZE)

  def __init__ (self, _n, _r, _s, _l):
    self.no   = _n
    self.ref  = _r
    self.status = _s
    self.line = _l
    self.completechunks = []

class Data:
  buoy  = None
  index = None
  logger = None


  # id for log
  me = ""

  def indexofbatch (self, id):
    n = 0
    for i in self.batches:
      if i.no == id:
        return n
      n = n + 1

    return None

  batches = [] # Known batches available on buoy, with flag indicating complete
               # download.

  indexf_uri  = None
  dataf_uri   = None
  indexf      = None # Index file for this data segment (refs + meta)
  dataf       = None # Samples with refs

  hasfull     = False # Has complete index been received
  hasalldata  = False # Has all data been received

  id            = 0
  enabled       = False
  samples       = 0
  refs_no       = 0

  def __init__ (self, l, _buoy, _index, _id, _enabled):
    self.logger = l
    self.buoy = _buoy
    self.index = _index
    self.id = _id
    self.enabled = _enabled
    self.me = "[" + self.buoy.name + "] [Data] [" + str(_id) + "]"


    self.indexf_uri = os.path.join (self.buoy.logdir, str(self.id) + '.ITT')
    self.dataf_uri  = os.path.join (self.buoy.logdir, str(self.id) + '.DTT')

    self.logger.debug (self.me + " Initializing (enabled: " + str(self.enabled) + ")")
    self.read_index ()
    self.read_data  ()

  ''' Read index file and figure out meta data and existing refs {{{

      Format (as returned by GETID):
      Index id
      Samples
      No of refs
      hasfull

      After this, one line for each batch:
      refno,ref,refstatus,line,list of completechunks
  '''
  def read_index (self):
    if self.enabled:
      if os.path.exists (self.indexf_uri):
        self.indexf = open (self.indexf_uri, 'r')

        self.id             = int(self.indexf.readline ())
        self.samples        = int(self.indexf.readline ())
        self.refs_no        = int(self.indexf.readline ())
        self.hasfull        = bool(self.indexf.readline ())

        for l in self.indexf.readlines ():
          s = l.split (',')
          b = Batch (int(s[0]), int(s[1]), int(s[2]), int(s[3]))

          c = []
          i = 4
          while i < len(s):
            c.append (int(s[i]))
            i = i + 1

          b.completechunks = c
          b.complete = (len(b.completechunks) == (BATCH_LENGTH / CHUNK_SIZE))

          self.batches.append (b)

        self.indexf.close ()
        self.batches = sorted (self.batches, key = lambda r: r.no)

      else:
        self.write_index ()

  def write_index (self):
    if self.enabled:
      self.batches = sorted (self.batches, key = lambda r: r.no)
      self.indexf = open (self.indexf_uri, 'w') # truncate file
      self.indexf.write (str(self.id) + '\n')
      self.indexf.write (str(self.samples) + '\n')
      self.indexf.write (str(self.refs_no) + '\n')
      self.indexf.write (str(self.hasfull) + '\n')
      for i in self.batches:
        self.indexf.write (str(i.no) + ',' + str(i.ref) + ',' + str(i.status)  + ',' + str(i.line))
        for c in i.completechunks:
          self.indexf.write (',' + str(c))

        self.indexf.write ('\n')

      self.indexf.close ()
  # }}}

  ''' Read data file and figure out which refs are complete '''
  def read_data (self):
    if self.enabled:
      if os.path.exists (self.dataf_uri):
        self.dataf = open (self.dataf_uri, 'r')

        # Scan for references
        n = 0
        for l in self.dataf.readlines ():
          if l[0] == 'R':
            s     = l.split (',')
            bl    = int(s[1])
            refno = int(s[2])
            ref   = int(s[3])
            stat  = int(s[4])

            self.logger.debug (self.me + " Found ref no: " + str(refno) + " at " + str(n))
            self.batches[self.indexofbatch (refno)].line = n

          n = n + 1 # line no

  def got_chunk (self, refno, start, length, reference, status, samples):
    if self.enabled:
      b = None
      i = self.indexofbatch (refno)

      first_on_batch = False

      # on new batch
      if i is None:
        b = Batch (refno, 0, 0, 0)
        self.batches.append (b)
        self.batches = sorted (self.batches, key = lambda r: r.no)
        first_on_batch = True
      else:
        b = self.batches[i]

      # mark chunk complete
      thischunk = start / CHUNK_SIZE
      b.completechunks.append (thischunk)

      # if first sample on ref, ref has been included
      if start == 0:
        b.ref    = reference
        b.status = status

      # write out updated data file
      lines = []
      if os.path.exists (self.dataf_uri):
        self.dataf = open (self.dataf_uri, 'r')
        lines = self.dataf.readlines ()
        self.dataf.close ()

      self.dataf = open (self.dataf_uri, 'w')
      n = 0
      thischunk_written = False

      for bb in self.batches:
        if bb is not b:
          self.dataf.write (lines[bb.line] + '\n') # write ref

          # write chunks
          for l in lines[bb.line+1:(bb.line+1 + len(bb.completechunks) * CHUNK_SIZE)]:
            self.dataf.write (l + '\n')

          n = n + 1 + len(bb.completechunks) * CHUNK_SIZE

          if thischunk_written:
            bb.line = bb.line + CHUNK_SIZE # update line no of batch after
                                           # new chunk.
            if first_on_batch:
              bb.line = bb.line + 1 # add line for ref if this is a new batch

        else:
          # on this batch
          b.line = n

          r = "R," + str(BATCH_LENGTH) + "," + str(b.no) + "," + str(b.ref) + "," + str(b.status)
          self.dataf.write (r + '\n')

          if len(b.completechunks) > 1:
            n = n + 1 # skip existing reference line

          for c in b.completechunks:
            if c == thischunk:
              # on this chunk
              for l in samples:
                self.dataf.writelines (str(l) + '\n')
              thischunk_written = True
            else:
              self.dataf.writelines (lines[n:n + CHUNK_SIZE])
              n = n + CHUNK_SIZE

      self.dataf.close ()

      # check if batch is complete
      b.complete = (len(b.completechunks) == (BATCH_LENGTH / CHUNK_SIZE))

      # check if datafile is complete
      if self.refs_no == len(self.batches):
        self.hasalldata = True
        for b in self.batches:
          self.hasalldata = (self.hasalldata and b.complete)

      else:
        self.hasalldata = False

      if self.hasalldata:
        self.logger.info (self.me + " All batches complete for data file.")

      # write indexes
      self.write_index ()

    else:
      self.logger.error (self.me + " Tried to append chunk on disabled data file.")

  def fullindex (self, _samples, n_refs):
    self.samples = _samples
    self.refs_no = n_refs
    self.hasfull = True

    # check if we have all refs and data
    if self.refs_no < len(self.batches):
      self.hasalldata = False

    self.write_index ()

  def __eq__ (self, other):
    return (self.id == other)

  def close (self):
    self.write_index ()

