# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-05-14
#
# index.py: Keep track of indexes for buoy

import threading
import logging
import time
import os

from data import *

class Index:
  buoy = None
  logger = None
  protocol = None

  indexf_uri = None
  indexf = None

  data = []
  greatestid = 0
  lastid = 0
  __incremental_id_check_done__ = False
  __unchecked_ids__             = None

  __full_data_check_done__      = False

  # id for log
  me = ""

  def __init__ (self, l, _buoy):
    self.logger = l
    self.buoy = _buoy
    self.protocol = self.buoy.protocol
    self.me = "[" + self.buoy.name + "] [Index]"
    self.indexf_uri = os.path.join (self.buoy.logdir, 'indexes')

    self.logger.info (self.me + " Initializing and opening index..")
    self.open_index ()

  ''' Open index list file and read known data segments and status {{{

      Index file format, text, one line per data segment (as returned by GETIDS):
      id,enabled
  '''
  def open_index (self):
    if os.path.exists (self.indexf_uri):
      self.indexf = open (self.indexf_uri, 'r')
      for l in self.indexf.readlines ():
        s = l.split (",")
        e = (s[1].strip () == "True")
        self.data.append (Data (self.logger, self.buoy, self, int(s[0]), e))

        if int(s[0]) > self.greatestid:
          self.greatestid = int(s[0])

      self.indexf.close ()

    self.data.sort (key = lambda d: d.id)

  ''' Write out all known indexes '''
  def write_index (self):
    self.indexf = open (self.indexf_uri, 'w+') # truncate file
    self.data.sort (key = lambda d: d.id)
    for i in self.data:
      self.indexf.write (str(i.id) + "," + str(i.enabled) + '\n')

    self.indexf.close ()
  # }}}

  def close (self):
    for i in self.data:
      i.close ()

    self.write_index ()

  ''' Update local list of ids from buoy, working backwards '''
  gotids_n = 0

  def getids (self, start):
    if self.state == 0:
      self.protocol.send ("GIDS," + str(start))
      self.request_t = time.time ()
      self.gotids_n = 0
      self.state    = 1

  def gotids (self, id, enabled):
    self.gotids_n = self.gotids_n + 1

    # check if id is in special check list
    if self.__unchecked_ids__ is not None:
      if id in self.__unchecked_ids__:
        self.__unchecked_ids__.remove (id)
        self.logger.debug (self.me + " Got missing id: " + str(id) + ", remaining: " + str(self.__unchecked_ids__))

    if id <= self.lastid:
      if enabled == 1:
        enabled = True
      else:
        enabled = False

      if self.greatestid < id:
        self.greatestid = id
        self.__full_data_check_done__ = False
        self.__incremental_id_check_done__ = False

      if self.indexofdata(id) is None:
        self.logger.info (self.me +  " Got id: " + str(id))
        self.data.append (Data (self.logger, self.buoy, self, id, enabled))
      else:
        self.logger.info (self.me + " Got already known id: " + str(id))

      self.write_index ()

    if self.gotids_n >= 10:
      if self.pendingid == 3:
        self.state = 0

  def indexofdata (self, id):
    n = 0
    for i in self.data:
      if i.id == id:
        return n
      n = n + 1

    return None

  def getlastid (self):
    if self.state == 0:
      self.request_t = time.time ()
      self.state = 1
      self.protocol.send ("GLID")

  def gotlastid (self, id):
    if id != self.lastid:
      self.__incremental_id_check_done__  = False
      self.__full_data_check__done__      = False

    self.lastid = id
    self.logger.info (self.me + " Latest id: " + str(self.lastid))
    self.sync_lastid_t = time.time ()
    if self.pendingid == 2:
      self.state = 0

  def getid (self, id):
    if self.state == 0:
      self.logger.info (self.me + " Getting full index for: " + str(id))
      self.pendingid = 4
      self.state     = 1

      self.protocol.send ("GID," + str(id))

      self.request_t = time.time ()

  # should probably be gotind..
  def gotid (self, id, samples, n_refs):
    self.logger.info (self.me + " Got full index: " + str(id) + ", samples: " + str(samples) + ", no of refs: " + str(n_refs))
    if self.working_data is not None:
      if self.working_data.id == id:
        self.working_data.fullindex (samples, n_refs)

    if self.pendingid == 4:
      self.state = 0

  def getbatch (self, id, ref, start, length):
    if self.state == 0:
      self.logger.info (self.me + "[" + str(id) + "] Req chunk, ref: " + str(ref) + ", start: " + str(start) + ", length: " + str(length))
      self.pendingid = 5
      self.state     = 1

      self.protocol.send ("GB," + str(id) + "," + str(ref) + "," + str(start) + "," + str(length))

      self.request_t = time.time ()

  def gotbatch (self, id, refno, start, length, ref, refstat, samples):
    self.logger.debug (self.me + " Got batch, id: " + str(id) + ", ref no: " + str(refno) + ", start: " + str(start) + ", length: " + str(length))
    if self.working_data.id == id:
      self.working_data.got_chunk (refno, start, length, ref, refstat, samples)

    if self.pendingid == 5:
      self.state = 0

  status = 0
  def getstatus (self):
    if self.state == 0:
      self.request_t = time.time ()
      self.status = 0
      self.state = 1
      self.protocol.send ("GS")
      self.pendingid = 1

  def gotstatus (self):
    self.sync_status_t = time.time ()
    if self.status == 0: # got gps status
      self.status = 1 # wait for ad status
    elif self.status == 1:
      if self.pendingid == 1:
        self.state  = 0
      self.status = 0
      self.sync_status_t = time.time ()
      self.logger.debug (self.me + " Status updated.")

  # State for keeping this buoys data uptodate
  state     = 0
  timeout   = 10 # secs
  pendingid = 0  # automatic request last sent (in case manual request mess up the flooooow..)
  request_t = 0

  sync_lastid   = 1 * 60 # time between syncs of last id
  sync_lastid_t = 0

  sync_status   = 20 # time between status updates
  sync_status_t = 0

  working_data  = None  # working data object, getting full index, refs and data

  def loop (self):
    # idle
    if self.buoy.zero.ser is not None and self.buoy.zero.acquire:
      if self.state == 0:
        # Get status and lastid {{{
        if time.time () - self.sync_status_t > self.sync_status:
          self.pendingid = 1
          self.getstatus ()
          return

        if time.time() - self.sync_lastid > self.sync_lastid_t:
          self.pendingid = 2
          self.getlastid ()
          return
        # }}}

        # check if we have all ids {{{
        if self.lastid > 0:
          # get ids from greatestid to lastid
          if self.greatestid < self.lastid:
            self.pendingid = 3
            self.getids (self.greatestid + 1)
            return

          # Get possibly missing ids
          elif not self.__incremental_id_check_done__:
            if self.__unchecked_ids__ is None:
              self.__unchecked_ids__ = []
              ii = 1
              while ii < self.greatestid:
                if self.indexofdata (ii) is None:
                  self.__unchecked_ids__.append (ii)
                ii = ii + 1

              self.logger.debug (self.me + " Getting missing ids: " + str(self.__unchecked_ids__))
              return

            else:
              if len(self.__unchecked_ids__) > 0:
                self.pendingid = 3
                self.getids (self.__unchecked_ids__[0]) # take first, but leave it in list, is removed when received
                return
              else:
                self.__unchecked_ids__              = None
                self.__incremental_id_check_done__  = True
                self.logger.debug (self.me + " All missing ids got.")
        # }}}

        # download data, strategy:
        #
        # start on last id, get full id then start to get batches and chunks
        # from beginning of corresponding data file to id.

        if self.working_data is not None:

          if not self.working_data.hasfull:
            # get full index
            self.getid (self.working_data.id)
            return

          elif not self.working_data.hasalldata:
            # continue to get refs on this id
            # figure out which refs are missing
            ii = 0
            while ii < self.working_data.refs_no:
              i = self.working_data.indexofbatch (ii)
              if i is None:
                # start on new batch
                self.getbatch (self.working_data.id, ii, 0, CHUNK_SIZE)
                return
              else:
                # check if this batch has been completed
                i = self.working_data.batches[i]
                if not i.complete:
                  # get rest of this batch, figure out which next chunk is missing
                  jj = 0
                  while jj < i.maxchunks:
                    if not jj in i.completechunks:
                      self.getbatch (self.working_data.id, ii, CHUNK_SIZE * jj, CHUNK_SIZE)
                      return
                    jj = jj + 1

                  # all chunks done on this batch

              ii = ii + 1

            # all batches done
            self.logger.info (self.me + " Finished id: " + str(self.working_data.id))
            self.working_data.hasalldata = True
            self.working_data.write_index ()
            self.working_data = None

          else:
            self.logger.info (self.me + " Finished id: " + str(self.working_data.id))
            self.working_data.hasalldata = True
            self.working_data.write_index ()
            self.working_data = None

        elif self.lastid > 0 and self.greatestid == self.lastid and not self.__full_data_check_done__:
          # find latest id with missing index, refs or data
          ii = self.lastid - 1

          while ii > 0:
            d  = None
            di = self.indexofdata (ii)
            if di is not None:
              d = self.data[di]

            if d is not None:
              if d.enabled and not (d.hasfull and d.hasalldata):
                self.working_data = d
                self.logger.info (self.me + " Working on id: " + str(d.id))
                return
              else:
                ii = ii - 1

          # none missing found
          self.working_data = None
          self.__full_data_check_done__ = True
          self.logger.info (self.me + " All data up to date.")
          return


      # waiting for response
      elif self.state == 1:
        # time.time out
        if time.time () - self.request_t > self.timeout:
          self.logger.debug (self.me + " Request timed out, reset..")
          self.reset ()

  def reset (self):
    self.logger.debug (self.me + " Resetting communication state.")
    # reset in case checksum mismatch or timeout
    self.state    = 0
    self.gotids_n = 0
    self.status   = 0
    self.pendingids = 0

    # reset protocol
    self.protocol.a_receive_state = 0
    self.protocol.a_buf = ''
    self.waitforreceipt = False


