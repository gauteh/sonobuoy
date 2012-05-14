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

  def __init__ (self, l, _buoy):
    self.logger = l
    self.buoy = _buoy
    self.indexf_uri = os.path.join (self.buoy.logdir, 'indexes')

    self.logger.info ("[Index] Initializing and opening index..")
    self.open_index ()

  ''' Open index list file and read known data segments and status

      Index file format, text, one line per data segment (as returned by GETIDS):
      id
  '''
  def open_index (self):
    if os.path.exists (self.indexf_uri):
      self.indexf = open (self.indexf_uri, 'r')
      while not self.indexf.eof ():
        l = self.indexf.readline ()
        self.data.append (Data (self.logger, self.buoy, self, int(l)))

      self.indexf.close ()

    self.data = sorted (self.data, key = lambda d: d.id)

  ''' Write out all known indexes '''
  def write_index (self):
    self.indexf = open (self.indexf_uri, 'w+') # truncate file
    self.data = sorted (self.data, key = lambda d: d.id)
    for i in self.data:
      self.indexf.writeline (str(i.id))

    self.indexf.close ()

  def close (self):
    for i in self.data:
      i.close ()

    self.write_index ()



