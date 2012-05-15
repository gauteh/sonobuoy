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

  indexf_uri = None
  indexf = None

  data = []
  greatestid = 0
  lastid = 0

  # id for log
  me = ""

  def __init__ (self, l, _buoy):
    self.logger = l
    self.buoy = _buoy
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
        if s[1].strip () == "True":
          e = True
        else:
          e = False

        self.data.append (Data (self.logger, self.buoy, self, int(s[0]), e))
        if int(s[0]) > self.greatestid:
          self.greatestid = int(s[0])

      self.indexf.close ()

    self.data = sorted (self.data, key = lambda d: d.id)

  ''' Write out all known indexes '''
  def write_index (self):
    self.indexf = open (self.indexf_uri, 'w+') # truncate file
    self.data = sorted (self.data, key = lambda d: d.id)
    for i in self.data:
      self.indexf.write (str(i.id) + "," + str(i.enabled) + '\n')

    self.indexf.close ()
  # }}}

  def close (self):
    for i in self.data:
      i.close ()

    self.write_index ()

  ''' Update local list of segments from buoy, going backwards '''
  gotids_n = 0
  def gotids (self, id, enabled):
    self.gotids_n = self.gotids_n + 1

    if id <= self.lastid:
      if enabled == 1:
        enabled = True
      else:
        enabled = False

      if self.greatestid < id:
        self.greatestid = id

      self.logger.info (self.me +  " Got id: " + str(id))
      if self.indexofdata(id) is None:
        self.data.append (Data (self.logger, self.buoy, self, id, enabled))
      else:
        self.logger.info (self.me + " Id already known: " + str(id))

      self.write_index ()

    if self.gotids_n >= 10:
      self.state = 0

  def indexofdata (self, id):
    n = 0
    for i in self.data:
      if i.id == id:
        return n
      n = n + 1

    return None

  def getlastid (self):
    self.buoy.getlastid ()

  def gotlastid (self, token):
    self.lastid = int(token)
    self.logger.info (self.me + " Latest id: " + str(self.lastid))
    self.sync_lastid_t = time.time ()
    self.state = 0

  def gotid (self):
    pass

  def gotrefs (self):
    pass

  def gotbatch (self):
    pass

  status = 0
  def gotstatus (self):
    if self.status == 0: # got gps status
      self.status = 1 # wait for ad status
    elif self.status == 1:
      self.state = 0
      self.sync_status_t = time.time ()
      self.logger.debug (self.me + " Status updated.")

  # State for keeping this buoys data uptodate
  state     = 0
  timeout   = 30 # secs
  request_t = 0

  sync_lastid   = 4 * 60 # time between syncs of last id
  sync_lastid_t = 0

  sync_status   = 20 # time between status updates
  sync_status_t = 0

  def loop (self):
    # idle
    if self.buoy.zero.ser is not None:
      if self.state == 0:
        if time.time () - self.sync_status_t > self.sync_status:
          self.buoy.getstatus ()
          self.request_t = time.time ()
          self.status = 0
          self.state = 1

        elif time.time() - self.sync_lastid > self.sync_lastid_t:
          self.getlastid ()
          self.request_t = time.time ()
          self.state = 1


        # check if we have all ids
        elif self.lastid > 0:
          # get ids down to greatestid
          if self.greatestid < self.lastid:
            self.request_t = time.time ()
            self.gotids_n = 0
            self.buoy.getids (self.greatestid + 1)
            self.state = 1


        # download data

      # waiting for response
      elif self.state == 1:
        # time.time out
        if time.time () - self.request_t > self.timeout:
          self.reset ()

  def reset (self):
    # reset in case checksum mismatch or timeout
    self.state    = 0
    self.gotids_n = 0
    self.status   = 0


