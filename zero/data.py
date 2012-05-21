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
  indexf_l    = threading.Lock ()
  dataf       = None # Samples with refs
  dataf_l     = threading.Lock ()

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
      self.indexf_l.acquire ()
      self.batches = sorted (self.batches, key = lambda r: r.no)
      self.indexf = open (self.indexf_uri, 'w') # truncate file
      self.indexf.truncate (0)
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
      self.indexf_l.release ()
  # }}}

  def got_chunk (self, refno, start, length, reference, status, samples):
    if self.enabled:
      self.dataf_l.acquire ()
      b = None
      i = self.indexofbatch (refno)

      fresh_batch = False

      if i is None:
        # on new batch
        b = Batch (refno, 0, 0, 0)
        self.batches.append (b)
        self.batches = sorted (self.batches, key = lambda r: r.no)
        fresh_batch = True
      else:
        # on existing batch
        b = self.batches[i]

      thischunk = start / CHUNK_SIZE

      # if first sample on ref, ref has been included
      if start == 0:
        b.ref    = reference
        b.status = status

      # read existing chunks
      lines = []
      if os.path.exists (self.dataf_uri):
        self.dataf = open (self.dataf_uri, 'r')
        lines = self.dataf.readlines ()
        self.dataf.close ()

      # write out updated data file
      self.dataf = open (self.dataf_uri, 'w')
      self.dataf.truncate (0)
      self.dataf.flush ()

      thischunk_written = False

      n = 0
      bi = 0
      while bi < len(self.batches):
        # on this batch
        if self.batches[bi].no == b.no:
          self.batches[bi].line = n

          # (re)-write ref
          r = "R," + str(BATCH_LENGTH) + "," + str(b.no) + "," + str(b.ref) + "," + str(b.status)
          self.dataf.write (r + '\n')
          
          if not fresh_batch:
            lines.pop (0)

          # write chunks
          ci = 0
          while ci < b.maxchunks:
            # write this chunk
            if not thischunk_written and thischunk == ci:
              k = 0
              while k < CHUNK_SIZE:
                self.dataf.write (str(samples.pop(0)) + '\n')
                k = k + 1

              thischunk_written = True

            # write existing chunks
            elif ci in b.completechunks:
              k = 0
              while k < CHUNK_SIZE:
                self.dataf.write (lines.pop (0))
                k = k + 1

            ci = ci + 1

        else:
          # on existing batch
          # write ref
          self.dataf.write (lines.pop (0))
          n = n + 1

          # write chunks
          ci = 0
          while ci < len(self.batches[bi].completechunks):
            k = 0
            while k < CHUNK_SIZE:
              self.dataf.write (lines.pop (0))
              n = n + 1
              k = k + 1

            ci = ci + 1

          if thischunk_written:
            self.batches[bi].line = self.batches[bi].line + CHUNK_SIZE
            if fresh_batch:
              self.batches[bi].line = self.batches[bi].line + 1

        bi = bi + 1

      self.dataf.flush ()
      self.dataf.close ()

      # mark chunk complete
      b.completechunks.append (thischunk)

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
      self.dataf_l.release ()

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

