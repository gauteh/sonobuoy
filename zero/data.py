# -*- coding: utf-8 -*-
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-05-14
#
# data.py: Keep track of one data file for buoy

import threading
import logging
import time
import os

BATCH_LENGTH = 1024
CHUNK_SIZE = 512

class Data:
  buoy  = None
  index = None
  logger = None


  # id for log
  me = ""


  class Batch:
    no      = 0   # no of ref/batch in data file
    ref     = None
    status  = None
    complete = False # Is batch completely received
    line    = None   # Where in data file does this ref start

    class Chunk:
      no        = None # 0 indexed
      line      = None # where in data file does this chunk start
                       # _must_ be ordered

    completechunks  = [] # list of chunks received
    maxchunks       = (BATCH_LENGTH / CHUNK_SIZE)

    def __init__ (self, _n, _r, _s, _l):
      self.no   = _n
      self.ref  = _r
      self.status = _s
      self.line = _l
      self.completechunks = []


  def indexofbatch (self, id):
    n = 0
    for i in self.batches:
      if i.id == id:
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
            t = Chunk ()
            t.no   = int(s[i])
            t.line = b.line + ((i - 4) * CHUNK_SIZE)
            c.append (t)

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
      self.indexf = open (self.indexf_uri, 'w+') # truncate file
      self.indexf.write (str(self.id) + '\n')
      self.indexf.write (str(self.samples) + '\n')
      self.indexf.write (str(self.refs_no) + '\n')
      self.indexf.write (str(self.hasfull) + '\n')
      for i in self.batches:
        self.indexf.write (str(i.no) + ',' + str(i.ref) + ',' + str(i.status)  + ',' + int(i.line))
        for c in i.completechunks:
          self.indexf.write (',' + str(c.no))

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
            bl    = s[1]
            refno = s[2]
            ref   = s[3]
            stat  = s[4]

            self.batches[self.indexofbatch (refno)].line = n
            self.logger.debug (self.me + " Found ref no: " + refno + " at " + str(n))

          n = n + 1 # line no

  def got_chunk (self, batch_length, reference, status, samples):
    if self.enabled:
      pass
    else:
      self.logger.error (self.me + " Tried to append batch on disabled data file.")

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

