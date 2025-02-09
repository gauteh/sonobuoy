# -*- coding: utf-8 -*-
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-05-14
#
# data.py: Keep track of one data file for buoy

import threading
import logging
import time
import os

BATCH_LENGTH = 1024 # samples per reference, as defined on buoy
CHUNK_SIZE   = 1024 # samples to get in each go

class Batch:
  no        = 0     # no of ref/batch in data file
  ref       = None
  status    = None
  latitude  = None
  longitude = None
  complete  = False # is batch completely received
  line      = None  # where in data file does this ref start


  completechunks  = None # list of chunks received
  maxchunks       = (BATCH_LENGTH / CHUNK_SIZE)

  def __init__ (self, _n, _r, _s, _lat, _lon, _crc, _l):
    self.no   = _n
    self.ref  = _r
    self.status = _s
    self.latitude  = _lat
    self.longitude = _lon
    self.checksum  = _crc
    self.line = _l
    self.completechunks = []

  def __repr__ (self):
    return str(self.no) + ", ref: " + str(self.ref) + ", completechunks: " + str(self.completechunks)

class Data:
  buoy    = None
  index   = None
  logger  = None

  # id for log
  me = ""

  def indexofbatch (self, id):
    n = 0
    for i in self.batches:
      if i.no == id:
        return n
      n = n + 1

    return None

  # Known batches available on buoy, with flag indicating complete
  # download.
  batches     = None

  indexf_uri  = None
  dataf_uri   = None
  indexf      = None # Index file for this data segment (refs + meta)
  indexf_l    = None
  dataf       = None # Samples with refs
  dataf_l     = None

  hasfull     = None # Has complete index been received
  hasalldata  = None # Has all data been received

  id          = None
  enabled     = None
  samples     = None
  refs_no     = None

  LASTVERSION   = 3
  localversion  = -1
  remoteversion = -1
  e_sdlag       = False

  def __init__ (self, l, _buoy, _index, _id, _enabled):
    self.logger   = l
    self.buoy     = _buoy
    self.index    = _index
    self.id       = _id
    self.enabled  = _enabled
    self.me = "[" + self.buoy.name + "] [Data] [" + str(_id) + "]"
    self.indexf_l = threading.Lock ()
    self.dataf_l  = threading.Lock ()

    self.indexf_uri = os.path.join (self.buoy.logdir, str(self.id) + '.ITT')
    self.dataf_uri  = os.path.join (self.buoy.logdir, str(self.id) + '.DTT')

    self.reset ()

    self.logger.debug (self.me + " Initializing (enabled: " + str(self.enabled) + ")")
    self.read_index ()

  def reset (self):
    self.hasfull    = False
    self.hasalldata = False
    self.samples = 0
    self.refs_no = 0
    if self.batches is not None:
      del (self.batches)
    self.batches = []

  ''' Read index file and figure out meta data and existing refs {{{

      Format:
      Local version
      Remote version
      Index id
      Samples
      No of refs
      hasfull
      E_SDLAG

      After this, one line for each batch:
      refno,ref,refstatus,latitude,longitude,checksum,line,list of completechunks
  '''
  def read_index (self):
    if self.enabled:
      if os.path.exists (self.indexf_uri):
        try:
          self.indexf = open (self.indexf_uri, 'r')

          self.localversion  = int(self.indexf.readline ())
          self.remoteversion = int(self.indexf.readline ())

          # Check if we are still on localversion = 1
          if self.remoteversion == 40960 or self.remoteversion == 0:
            self.id       = self.localversion
            self.samples  = self.remoteversion

            self.localversion   = 1
            self.remoteversion  = 6

          else:
            self.id             = int(self.indexf.readline ())
            self.samples        = int(self.indexf.readline ())

          self.refs_no        = int(self.indexf.readline ())
          r = self.indexf.readline ().strip ()
          self.hasfull        = (r == "True")

          if self.localversion >= 3:
            r = self.indexf.readline ().strip ()
            self.e_sdlag = (r == "True")

          for l in self.indexf.readlines ():
            l = l.strip()
            s = l.split (',')
            b = Batch (int(s[0]), int(s[1]), int(s[2]), s[3], s[4], int(s[5]), int(s[6]))

            c = []
            i = 7
            while i < len(s):
              c.append (int(s[i]))
              i = i + 1

            b.completechunks = c
            b.complete = (len(b.completechunks) == (BATCH_LENGTH / CHUNK_SIZE))

            self.batches.append (b)

          self.indexf.close ()

          self.batches.sort (key=lambda r: r.no)

          self.check_hasalldata ()

        except Exception as e:
          self.logger.error (self.me + " Error while opening data index..")
          self.logger.exception (e)
          self.reset_data ()
      else:
        self.write_index ()

  def write_index (self):
    if self.enabled:
      self.indexf_l.acquire ()
      self.batches.sort (key=lambda r: r.no)
      self.indexf = open (self.indexf_uri, 'w') # truncate file
      self.indexf.truncate (0)
      self.localversion = self.LASTVERSION
      self.indexf.write (str(self.localversion) + '\n')
      self.indexf.write (str(self.remoteversion) + '\n')
      self.indexf.write (str(self.id) + '\n')
      self.indexf.write (str(self.samples) + '\n')
      self.indexf.write (str(self.refs_no) + '\n')
      self.indexf.write (str(self.hasfull) + '\n')
      self.indexf.write (str(self.e_sdlag) + '\n')

      for i in self.batches:
        self.indexf.write (str(i.no) + ',' + str(i.ref) + ',' + str(i.status)  + ',' + i.latitude + "," + i.longitude + "," + str(i.checksum) + ',' + str(i.line))
        for c in i.completechunks:
          self.indexf.write (',' + str(c))

        self.indexf.write ('\n')

      self.indexf.close ()
      self.indexf_l.release ()
  # }}}

  def got_chunk (self, refno, start, length, reference, status, latitude, longitude, checksum, samples):
    if self.enabled:
      self.logger.info (self.me + " Got chunk, ref: " + str(refno) + ", start: " + str(start) + ", length: " + str(length))

      self.dataf_l.acquire ()

      try:
        b = None
        i = self.indexofbatch (refno)

        fresh_batch = False

        thischunk = start / CHUNK_SIZE

        if i is None:
          # on new batch
          b = Batch (refno, 0, 0, 0, 0, 0, 0)
          self.batches.append (b)
          self.batches.sort (key=lambda r: r.no)
          fresh_batch = True
        else:
          # on existing batch
          b = self.batches[i]

          if thischunk in b.completechunks:
            self.logger.error (self.me + " Chunk already exists on reference, discarding.")
            self.dataf_l.release ()
            #self.reset_data ()
            return


        # if first sample on ref, ref has been included
        if start == 0:
          b.ref    = reference
          b.status = status
          b.latitude = latitude
          b.longitude = longitude

          try:
            _t = float (b.latitude[:-1])
            _t = float (b.longitude[:-1])

            if b.latitude[-1] is not 'N' and b.latitude[-1] is not 'S':
              raise ValueError

            if b.longitude[-1] is not 'E' and b.longitude[-1] is not 'W':
              raise ValueError

          except ValueError:
            b.latitude  = '0S'
            b.longitude = '0W'

          b.checksum = checksum

        # read existing chunks
        lines = []
        if os.path.exists (self.dataf_uri):
          self.dataf = open (self.dataf_uri, 'r')
          lines = self.dataf.readlines ()
          self.dataf.close ()

        # write out updated data file
        self.dataf = open (self.dataf_uri, 'w')
        self.dataf.truncate (0)

        thischunk_written = False

        n = 0
        bi = 0
        while bi < len(self.batches):
          # on this batch
          if self.batches[bi].no == b.no:
            self.batches[bi].line = n

            # (re)-write ref
            r = "R," + str(BATCH_LENGTH) + "," + str(b.no) + "," + str(b.ref) + "," + str(b.status) + "," + str(b.latitude) + "," + str(b.longitude) + "," + str(b.checksum)
            self.dataf.write (r + '\n')

            # throw away old ref
            if not fresh_batch and len(b.completechunks) > 0:
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

        self.dataf.close ()

        # mark chunk complete
        b.completechunks.append (thischunk)

        # check if batch is complete
        b.complete = (len(b.completechunks) == (BATCH_LENGTH / CHUNK_SIZE))

        # check if datafile is complete
        self.check_hasalldata ()

        if self.hasalldata:
          self.logger.info (self.me + " All batches complete for data file.")

        # write indexes
        self.write_index ()

        self.dataf_l.release ()

      except Exception as e:
        self.dataf_l.release ()
        self.logger.error (self.me + " Error while receiving and writing chunk, resetting..")
        self.logger.exception (e)
        self.reset_data ()

    else:
      self.logger.error (self.me + " Tried to append chunk on disabled data file.")

  def fullindex (self, _samples, n_refs, store_version, _e_sdlag):
    self.samples = _samples
    self.refs_no = n_refs
    self.remoteversion = store_version
    self.hasfull = True
    self.e_sdlag = _e_sdlag

    # check if we have all refs and data
    self.check_hasalldata ()

    self.write_index ()

  def check_hasalldata (self):
    if self.refs_no == len(self.batches):
      self.hasalldata = True
      for b in self.batches:
        self.hasalldata = (self.hasalldata and b.complete)

    else:
      self.hasalldata = False

    self.hasalldata = (self.hasalldata and self.hasfull)

  def reset_data (self):
    self.dataf_l.acquire ()

    self.logger.error (self.me + " Deleting index and data files and resetting..")

    # remove index and data file and reload
    try:
      os.unlink (self.dataf_uri)
    except: pass
    try:
      os.unlink (self.indexf_uri)
    except: pass

    self.reset ()
    self.read_index ()

    self.dataf_l.release ()

  def __eq__ (self, other):
    return (self.id == other)

  def close (self):
    pass
    # self.write_index ()

