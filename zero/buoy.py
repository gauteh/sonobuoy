# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-26
#
# buoy.py: Represents one Buoy with AD and GPS

import threading
import logging
import time
import os

from ad import *
from gps import *

class Buoy:
  zero = None

  gps  = None
  ad   = None

  id      = -1    # Unique ID
  name    = ''    # Unique friendly name
  BASEDIR = 'log'
  logdir  = ''    # Will be BASEDIR/name
  logfile = ''
  logfilef = None

  keeprun = True
  active  = False
  runthread = None
  logger  = None

  LOG_TIME_DELAY = 2

  def __init__ (self, z, id, n):
    self.zero   = z
    self.id     = id
    self.name   = n
    self.logdir = os.path.join (self.BASEDIR, self.name)

    if not os.path.exists (self.logdir):
      os.makedirs (self.logdir)

    self.logfile  = os.path.join (self.logdir, self.name + '.dtt')
    self.logger   = self.zero.logger

    self.logger.info ('[' + self.name + '] Initializing Buoy..')

    self.gps  = Gps (self)
    self.ad   = AD (self)

    # Open file
    self.logfilef = open (self.logfile, 'a')

    # Starting log thread
    self.runthread = threading.Thread (target = self.run, name = 'Buoy' + self.name)
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
    self.logger.info ("[" + self.name + "] Stopping..")
    self.keeprun = False
    self.runthread.join ()
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

    self.logger.info ("[" + self.name + "] Stopped.")

