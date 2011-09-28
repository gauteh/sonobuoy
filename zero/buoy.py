# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-26
#
# buoy.py: Represents one Buoy with AD and GPS

import threading
import logging
import time

from ad import *
from gps import *

class Buoy:
  zero = None

  gps  = None
  ad   = None

  node = ''
  logfile = '' 
  logfilef = None

  keeprun = True
  active  = False
  runthread = None
  logger  = None

  LOG_TIME_DELAY = 2

  def __init__ (self, z, n):
    self.zero = z
    self.node = n
    self.logfile = self.node + '.log'
    self.logger = self.zero.logger
    self.logger.info ('Starting Buoy ' + self.node + '..')

    self.gps = Gps (self)
    self.ad = AD7710 (self)

    # Open file
    self.logfilef = open (self.logfile, 'a')

    self.name = 'Buoy' + self.node

    # Starting log thread 
    self.runthread = threading.Thread (target = self.run, name = 'Buoy' + self.node )
    self.runthread.start ()

  def log (self):
    self.logger.debug ('[' + self.name + '] Writing data file.. (every ' + str(self.LOG_TIME_DELAY) + ' seconds)')

    # Acquire lock and swap stores
    self.ad.storelock.acquire ()
    self.ad.swapstore ()
    self.ad.storelock.release ()

    # Write data from inactive store
    l = len(self.ad.samplesb) if (self.ad.store == 0) else len(self.ad.samplesa)
    i = 0
    while i < l:
      if self.ad.store == 0:
        self.logfilef.write (str(self.ad.samplesb[i][0]) + ',' + str(self.ad.samplesb[i][1]) + '\n')
      else:
        self.logfilef.write (str(self.ad.samplesa[i][0]) + ',' + str(self.ad.samplesa[i][1]) + '\n')

      i += 1

    # Clear inactive store 
    if self.ad.store == 0:
      self.ad.samplesb = []
    else:
      self.ad.samplesa = []

    self.logfilef.flush ()

  def stop (self):
    self.keeprun = False
    self.log ()
    self.logfilef.close ()


  def activate (self):
    self.active = True

  def deactivate (self):
    self.active = False

  def run (self):
    i = self.LOG_TIME_DELAY # Log on first iteration

    while self.keeprun:
      if self.active:
        if (i >= self.LOG_TIME_DELAY):
          self.log ()
          i = 0

        i += 0.1

      time.sleep (0.1)

